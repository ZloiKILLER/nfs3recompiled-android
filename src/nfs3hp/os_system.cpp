#include "os_system.h"
#include "os_window.h"

#include <SDL3/SDL.h>
#include <cctype>
#include <cstring>
#include <lib/file.h>
#include <lib/library.h>
#include <lib/memmap.h>
#include <lib/process.h>
#include <lib/settings.h>
#include <new>

namespace nfs3hp
{
namespace os
{

void release(App* app, Handle handle)
{
    if (handle != 0)
        app->freeResource(handle);
}

void exitProcess(App* app, nfs::ThreadState& ts, nfs::u32 code)
{
    // Said, with the game's code that asked for it (the window's quit has the same).
    SDL_Log("[EXIT] exitProcess(%u) from 0x%x", unsigned(code), unsigned(app->getMemory<nfs::u32>(ts.sp)));
    window::quit();  // the window's thread told to stop
    ts.terminate = true;
}

void debugBreak()
{
    NFS2_ASSERT(false);
}

nfs::u32 commandLine(App* app)
{
    return app->getAppName();
}

nfs::u32 environment(App* app)
{
    return app->getEnv();
}

std::string programPath(App* app)
{
    return std::string("C:\\") + app->getAppNameRaw();
}

Handle currentProcess(App* app)
{
    return app->allocateResource(new nfs::Process());
}

Handle standardStream(App* app, int stream)
{
    static const char* const kNames[] = {"stdin", "stdout", "stderr"};
    return app->allocateResource(new nfs::File(stream, kNames[stream]));
}

namespace modules
{

std::optional<Handle> load(App* app, const char* name)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading library %s", name);
    nfs::LibraryHandle* library = new nfs::LibraryHandle(name);
    if (!*library)
    {
        delete library;
        return std::nullopt;
    }
    return app->allocateResource(library);
}

nfs::u32 symbol(App* app, Handle library, const char* name)
{
    nfs::LibraryHandle* handle = dynamic_cast<nfs::LibraryHandle*>(app->getResource(library));
    nfs::Library* built = handle ? handle->getLibrary() : nullptr;
    if (!built)
        return 0;
    const nfs::Library::Symbol found = (*built)[name];
    if (!found.first)
        SDL_Log("GetProcAddress: no %s in the port", name);
    return found.first;
}

}  // namespace modules

namespace memory
{

nfs::u32 allocate(App* app, nfs::u32 bytes)
{
    nfs::MemMap* map = nullptr;
    try
    {
        map = new nfs::MemMap(bytes);
    }
    catch (const std::bad_alloc&)
    {
        SDL_Log("[API] VirtualAlloc failed: size=%u", unsigned(bytes));
        return 0;
    }
    std::memset(&app->getMemory<char>(map->getBlockStart()), 0, bytes);
    if (App::traceApi())
        SDL_Log("[API] VirtualAlloc size=%u -> 0x%x..0x%x", unsigned(bytes), unsigned(map->getBlockStart()),
                unsigned(map->getBlockStart() + map->getBlockSize()));
    return map->getBlockStart();
}

void release(nfs::u32 address)
{
    if (address == 0)
        return;
    nfs::MemMap* block = nfs::MemMap::findBlock(address);
    if (App::traceApi())
        SDL_Log("[API] VirtualFree addr=0x%x -> block 0x%x..0x%x", unsigned(address),
                block ? unsigned(block->getBlockStart()) : 0u,
                block ? unsigned(block->getBlockStart() + block->getBlockSize()) : 0u);
    delete block;
}

Region regionOf(nfs::u32 address)
{
    const nfs::MemMap* block = nfs::MemMap::findBlock(address);
    return Region{block->getBlockStart(), block->getBlockSize()};
}

}  // namespace memory

namespace text
{

void upper(char* text)
{
    for (char* c = text; *c; ++c)
        *c = char(std::toupper(*c));
}

void upper(char* text, nfs::u32 length)
{
    for (nfs::u32 i = 0; i < length; ++i)
        text[i] = char(std::toupper(text[i]));
}

int widen(const char* from, int count, nfs::u16* to)
{
    int taken = 0;
    if (to)
        *to++ = 0xfffe;
    for (int i = 0; i < count; ++i)
    {
        taken++;
        // The bytes are gone through only as they are written: with no `to`, the first one is
        // looked at again each time.
        if (to)
            *to++ = nfs::u16(nfs::u8(*from++));
        if (!*from)
            break;
    }
    return taken;
}

}  // namespace text

void messageBox(const char* text, const char* caption, Box look)
{
    const SDL_MessageBoxFlags flags = look == Box::error     ? SDL_MESSAGEBOX_ERROR
                                      : look == Box::warning ? SDL_MESSAGEBOX_WARNING
                                                             : SDL_MessageBoxFlags(0);
    SDL_ShowSimpleMessageBox(flags, caption, text, nullptr);
}

void debugText(const char* text)
{
    SDL_Log("%s", text);
}

namespace settings
{

namespace
{

nfs::SettingsRoot rootKey(Root root)
{
    switch (root)
    {
    case Root::classes:
        return nfs::SettingsRoot::classes;
    case Root::currentUser:
        return nfs::SettingsRoot::currentUser;
    case Root::localMachine:
        break;
    }
    return nfs::SettingsRoot::localMachine;
}

nfs::SettingsKey* keyOf(App* app, Root root, const char* path)
{
    nfs::SettingsKey* key = dynamic_cast<nfs::SettingsKey*>(app->getResource(nfs::u32(rootKey(root))));
    for (const char* part = path; key && *part;)
    {
        const char* end = part;
        while (*end && *end != '\\')
            ++end;
        key = key->getChild(std::string(part, end));
        part = *end ? end + 1 : end;
    }
    return key;
}

}  // namespace

bool has(App* app, Root root, const char* key)
{
    return keyOf(app, root, key) != nullptr;
}

std::optional<Setting> find(App* app, Root root, const char* key, const char* name)
{
    nfs::SettingsKey* found = keyOf(app, root, key);
    nfs::SettingValue* value = found ? found->getValue(name) : nullptr;
    if (!value)
        return std::nullopt;
    Setting setting;
    setting.number = value->isNumber();
    if (setting.number)
        setting.value = value->number();
    else
        setting.text = value->text();
    return setting;
}

nfs::u32 copyTo(App* app, const Setting& setting, nfs::u32 to, nfs::u32 room)
{
    if (setting.number)
    {
        if (room >= 4)
            std::memcpy(&app->getMemory<char>(to), &setting.value, 4);
        return 4;
    }
    const nfs::u32 bytes = nfs::u32(setting.text.size() + 1);
    if (room >= bytes)
        std::memcpy(&app->getMemory<char>(to), setting.text.c_str(), bytes);
    return bytes;
}

}  // namespace settings

namespace
{

/* What the version string can hold, its ending with it. */
constexpr nfs::u32 kVersionRoom = kVersionBytes - kVersionText;

std::string& versionString()
{
    static std::string value;
    return value;
}

}  // namespace

void setFileVersion(const char* value)
{
    versionString() = value ? value : "";
    if (versionString().size() >= kVersionRoom)
        versionString().resize(kVersionRoom - 1);
}

const std::string& fileVersion()
{
    return versionString();
}

std::optional<std::string> versionValue(const char* key)
{
    static const char kFileVersion[] = "\\FileVersion";
    const size_t length = key ? std::strlen(key) : 0;
    const size_t suffix = sizeof(kFileVersion) - 1;
    if (versionString().empty() || length < suffix || SDL_strcasecmp(key + length - suffix, kFileVersion) != 0)
        return std::nullopt;
    return versionString();
}

}  // namespace os
}  // namespace nfs3hp
