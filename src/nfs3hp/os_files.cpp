#include "os_files.h"

#include <SDL3/SDL.h>
#include <cstring>
#include <lib/file.h>
#include <lib/memmap.h>
#include <new>

namespace nfs3hp
{
namespace os
{
namespace files
{

namespace
{

/* src/lib's File takes its mode and disposition as its own. */
nfs::File::Mode fileMode(Access access)
{
    switch (access)
    {
    case Access::read:
        return nfs::File::Mode::read;
    case Access::write:
        return nfs::File::Mode::write;
    case Access::readWrite:
        break;
    }
    return nfs::File::Mode::readWrite;
}

nfs::File::Disposition fileDisposition(Create create)
{
    switch (create)
    {
    case Create::openExisting:
        return nfs::File::Disposition::openExisting;
    case Create::createNew:
        return nfs::File::Disposition::createNew;
    case Create::createAlways:
        return nfs::File::Disposition::createAlways;
    case Create::openAlways:
        return nfs::File::Disposition::openAlways;
    case Create::truncateExisting:
        break;
    }
    return nfs::File::Disposition::truncateExisting;
}

nfs::File* fileOf(App* app, Handle handle)
{
    return dynamic_cast<nfs::File*>(app->getResource(handle));
}

void found(const nfs::FileEntry& entry, Found& out)
{
    out.name = entry.name;
    out.directory = entry.directory;
    out.size = entry.size;
}

}  // namespace

std::optional<Handle> open(App* app, const char* path, Access access, Create create)
{
    nfs::File* file = new nfs::File(path, fileMode(access), fileDisposition(create));
    if (!*file)
    {
        if (App::traceApi())
            SDL_Log("[API] open '%s' name@0x%x access=%d -> FAILED", path, unsigned(app->guestAddress(path)),
                    int(access));
        delete file;
        return std::nullopt;
    }
    const Handle handle = app->allocateResource(file);
    if (App::traceApi())
        SDL_Log("[API] open '%s' name@0x%x access=%d -> handle=%u", path, unsigned(app->guestAddress(path)),
                int(access), unsigned(handle));
    return handle;
}

std::optional<Handle> openRead(App* app, const char* path)
{
    return open(app, path, Access::read, Create::openExisting);
}

bool read(App* app, nfs::ThreadState& ts, Handle file, void* buffer, nfs::u32 length, nfs::u32& done)
{
    nfs::File* f = fileOf(app, file);
    if (!f)
    {
        if (App::traceApi())
            SDL_Log("[API] read h=%u requested=%u -> not a file", unsigned(file), unsigned(length));
        return false;
    }
    app->unlockContext(ts);
    const bool read = f->read(buffer, length, &done) != 0;
    app->lockContext(ts);
    if (App::traceApi())
        SDL_Log("[API] read h=%u requested=%u -> ok=%d read=%u", unsigned(file), unsigned(length), int(read),
                unsigned(done));
    return read;
}

bool write(App* app, nfs::ThreadState& ts, Handle file, const void* buffer, nfs::u32 length, nfs::u32& done)
{
    nfs::File* f = fileOf(app, file);
    if (!f)
        return false;
    app->unlockContext(ts);
    const bool written = f->write(buffer, length, &done) != 0;
    app->lockContext(ts);
    return written;
}

std::optional<nfs::u32> seek(App* app, Handle file, nfs::s32 offset, From from)
{
    nfs::File* f = fileOf(app, file);
    const nfs::u32 place = f ? f->seek(nfs::u32(from), offset) : 0xffffffff;
    if (App::traceApi())
        SDL_Log("[API] seek h=%u from=%d distance=%d -> %u", unsigned(file), int(from), int(offset), unsigned(place));
    if (place == 0xffffffff)
        return std::nullopt;
    return place;
}

std::optional<nfs::u32> size(App* app, Handle file)
{
    nfs::File* f = fileOf(app, file);
    if (!f)
        return std::nullopt;
    return f->size();
}

bool truncate(App* app, Handle file)
{
    nfs::File* f = fileOf(app, file);
    return f && f->truncate() != 0;
}

Copy copyToMemory(App* app, nfs::ThreadState& ts, Handle file, nfs::u32 length)
{
    // The holder: the file opened once more, to be read.
    nfs::File* holder = new nfs::File(*fileOf(app, file));
    Copy copy{app->allocateResource(holder), 0};
    app->unlockContext(ts);
    nfs::MemMap* block = nullptr;
    try
    {
        block = new nfs::MemMap(length);
    }
    catch (const std::bad_alloc&)
    {
        app->lockContext(ts);
        return copy;
    }
    void* data = &app->getMemory<char>(block->getBlockStart());
    holder->seek(0, 0);
    nfs::u32 done = 0;
    holder->read(data, length, &done);
    NFS2_ASSERT(done == length);
    app->lockContext(ts);
    copy.memory = block->getBlockStart();
    if (App::traceApi())
        SDL_Log("[API] copyToMemory h=%u requested=%u read=%u -> addr=0x%x", unsigned(file), unsigned(length),
                unsigned(done), unsigned(copy.memory));
    return copy;
}

bool remove(const char* path)
{
    return nfs::File::remove(path) != 0;
}

bool makeDirectory(const char* path)
{
    return path && nfs::File::createDirectory(path) != 0;
}

bool removeDirectory(const char* path)
{
    NFS2_USE(path);
    NFS2_ASSERT(false);
    return false;
}

void setCurrentDirectory(const char* path)
{
    /* Worth a line: the value is remembered but relative paths do not resolve against it, so
     * anything the game reaches by setting a directory and then using a bare name looks in the
     * data root instead. */
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SetCurrentDirectory %s", path ? path : "(null)");
    nfs::File::setCurrentDirectory(path);
}

const std::string& currentDirectory()
{
    return nfs::File::getCurrentDirectory();
}

bool isFile(const char* path)
{
    nfs::File file(path, nfs::File::Mode::read, nfs::File::Disposition::openExisting);
    return file;
}

std::optional<Handle> findFirst(App* app, const char* pattern, Found& out)
{
    nfs::FileEnumerator* search = new nfs::FileEnumerator(pattern);
    nfs::FileEntry data;
    if (!search->findNext(data))
    {
        delete search;
        return std::nullopt;
    }
    found(data, out);
    return app->allocateResource(search);
}

bool findNext(App* app, Handle search, Found& out)
{
    nfs::FileEnumerator* enumerator = dynamic_cast<nfs::FileEnumerator*>(app->getResource(search));
    nfs::FileEntry data;
    if (!enumerator->findNext(data))
        return false;
    found(data, out);
    return true;
}

FullPath fullPath(const char* name)
{
    static const char kRoot[] = "C:\\nfs2se\\";
    return FullPath{std::string(kRoot) + name, nfs::u32(sizeof kRoot - 1)};
}

Drive drive(char letter)
{
    switch (letter)
    {
    case 'C':
        return Drive::disk;
    case 'D':
        return Drive::disc;
    default:
        return Drive::none;
    }
}

}  // namespace files
}  // namespace os
}  // namespace nfs3hp
