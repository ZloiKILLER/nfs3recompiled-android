#ifndef NFS3HP_OS_FILES_H
#define NFS3HP_OS_FILES_H

#include "os_system.h"

#include <optional>
#include <string>

/* The port's files, directories and drives, as the game's code needs them, on src/lib's File:
 * paths as the game writes them (a drive's X:\ or a leading \ the data's root, \ or /, any case),
 * found whatever the case of the files on the device.  A file is held by a handle (os_system.h) the game keeps
 * in its memory; reads and writes let the other threads run while they wait. */
namespace nfs3hp
{
namespace os
{
namespace files
{

/* What a file is opened for. */
enum class Access
{
    read,
    write,
    readWrite,
};

/* What an open does with a file that is there, or is not. */
enum class Create
{
    openExisting,      // opened if it is there, else not
    createNew,         // made if it is not there, opened if it is
    createAlways,      // made, or emptied if it is there
    openAlways,        // opened, made if it is not there
    truncateExisting,  // emptied if it is there, else not
};

/* The file at `path` opened as `access` and `create` say; nullopt when it cannot be (a directory
 * never opens). */
std::optional<Handle> open(App* app, const char* path, Access access, Create create);

/* The file at `path` opened to be read; nullopt when it is not there. */
std::optional<Handle> openRead(App* app, const char* path);

/* Up to `length` bytes of `file` into `buffer`, how many to `done`: false when `file` is not a
 * file, or nothing could be read. */
bool read(App* app, nfs::ThreadState& ts, Handle file, void* buffer, nfs::u32 length, nfs::u32& done);

/* `length` bytes from `buffer` to `file`, how many to `done`: false when `file` is not a file, or
 * not all could be written. */
bool write(App* app, nfs::ThreadState& ts, Handle file, const void* buffer, nfs::u32 length, nfs::u32& done);

/* Where a seek counts from. */
enum class From
{
    start,
    current,
    end,
};

/* `file`'s place moved `offset` bytes from where `from` says: the place it has then; nullopt
 * when `file` is not a file or cannot be moved there. */
std::optional<nfs::u32> seek(App* app, Handle file, nfs::s32 offset, From from);

/* How long `file` is; nullopt when it is not a file. */
std::optional<nfs::u32> size(App* app, Handle file);

/* `file` cut where its place is: false when it cannot be. */
bool truncate(App* app, Handle file);

/* A file's copy in memory: the handle that holds it open (a second reading of the file), and
 * where the copy is in the game's memory -- 0 when there was not the memory for it. */
struct Copy
{
    Handle holder;
    nfs::u32 memory;
};

/* The first `length` bytes of `file` copied into memory of their own; a copy changed is not
 * written back.  The memory goes back by memory::release, the holder by release. */
Copy copyToMemory(App* app, nfs::ThreadState& ts, Handle file, nfs::u32 length);

/* The file at `path` deleted: false when it cannot be. */
bool remove(const char* path);

/* The directory at `path` made, its parent already there: false when it cannot be, or is there
 * already. */
bool makeDirectory(const char* path);

/* The port removes no directory: false, always. */
bool removeDirectory(const char* path);

/* The current directory: set to `path`, and as it is (with a \ at its end).  Relative paths are
 * not resolved against it: they start at the data's root. */
void setCurrentDirectory(const char* path);
const std::string& currentDirectory();

/* Whether there is a file at `path` that opens to be read (a directory does not).  The port keeps
 * no attributes: such a file is reported read-only. */
bool isFile(const char* path);

/* A file a search finds: its name (no directory), whether it is a directory, its size. */
struct Found
{
    std::string name;
    bool directory = false;
    nfs::u32 size = 0;
};

/* A search of the files `pattern` matches (* and ? in its last part, any case), the first one
 * found to `found`: the search's handle, or nullopt when nothing matched -- no search then. */
std::optional<Handle> findFirst(App* app, const char* pattern, Found& found);

/* The search `search`'s next file to `found`: false when there is no more. */
bool findNext(App* app, Handle search, Found& found);

/* The full path the port gives for `name`: C:\nfs2se\ and `name` as it is; where the name starts
 * in it. */
struct FullPath
{
    std::string path;
    nfs::u32 nameAt;
};
FullPath fullPath(const char* name);

/* The drives the port has: C: a disk, D: the disc. */
enum class Drive
{
    none,
    disk,
    disc,
};
Drive drive(char letter);

/* The drives' list as the port gives it -- their letters, "CD", one string -- and their bits
 * (1 << n for drive n, 0 A:) as it gives them, 0x6. */
constexpr const char* kDriveList = "CD";
constexpr nfs::u32 kDriveBits = 0x6;

/* What a volume is said to be: its clusters, by sectors of bytes; its longest name; whether it
 * keeps names' case and has Unicode names (both, always). */
struct Volume
{
    nfs::u32 sectorsPerCluster = 1;
    nfs::u32 bytesPerSector = 512;
    nfs::u32 freeClusters = 512 * 1024;
    nfs::u32 totalClusters = 1024 * 1024;
    nfs::u32 longestName = 255;
    bool keepsCase = true;
    bool unicodeNames = true;
};
constexpr Volume kVolume{};

}  // namespace files
}  // namespace os
}  // namespace nfs3hp

#endif
