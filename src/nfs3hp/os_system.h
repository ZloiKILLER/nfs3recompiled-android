#ifndef NFS3HP_OS_SYSTEM_H
#define NFS3HP_OS_SYSTEM_H

#include <game_types.h>
#include <lib/app.h>

#include <optional>
#include <string>

/* The port's own system services, as the game's code needs them: the objects it holds by handle,
 * the process (leaving, its name, its command line and environment, the standard streams), the
 * memory pages it takes, the modules it loads by name, its text (capitals, the code page, wide
 * characters), message boxes and the debugger's log, the settings it reads (what was the
 * registry), and its executable's version.  Plain C++ on src/lib's application, MemMap, Library
 * and SettingsKey; the game's code keeps in its own memory only what it stores there itself. */
namespace nfs3hp
{
namespace os
{

using App = nfs::App;

/* An object the port holds for the game -- a file, a search, a file's copy in memory, a thread,
 * an event, a library -- by its place in the application's table: the number the game keeps in
 * its memory. */
using Handle = nfs::u32;

/* The object `handle` let go; 0 is none. */
void release(App* app, Handle handle);

/* ---- The process ---- */

/* The game ends: said in the log with the code that asked, and the game's thread stops
 * (ts.terminate). */
void exitProcess(App* app, nfs::ThreadState& ts, nfs::u32 code);

/* A stop for a debugger: the port logs it and goes on. */
void debugBreak();

/* The command line, the program's name, in the game's memory: its address. */
nfs::u32 commandLine(App* app);

/* The environment's block in the game's memory: its address. */
nfs::u32 environment(App* app);

/* The path the game is given for its own executable: C:\ and its name. */
std::string programPath(App* app);

/* The module handle the game is given for its own executable. */
constexpr Handle kProgramModule = 1;

/* A handle of the process itself, a new one each time. */
Handle currentProcess(App* app);

/* The standard streams, 0 input, 1 output, 2 errors: a file handle of a copy of each. */
Handle standardStream(App* app, int stream);

/* ---- Modules ---- */

namespace modules
{

/* The library `name`, as the port has it built in; nullopt when it has none of that name. */
std::optional<Handle> load(App* app, const char* name);

/* The address of `name` in the library `library` (load's); 0 when either is not there. */
nfs::u32 symbol(App* app, Handle library, const char* name);

}  // namespace modules

/* ---- Memory ---- */

namespace memory
{

/* The size of the pages the game is told of. */
constexpr nfs::u32 kPageSize = 4096;

/* `bytes` of the game's memory, cleared: their address, 0 when there is not that much. */
nfs::u32 allocate(App* app, nfs::u32 bytes);

/* The memory at `address` (allocate's, or a file's copy) given back. */
void release(nfs::u32 address);

/* The block of the game's memory `address` is in: where it starts and how long it is. */
struct Region
{
    nfs::u32 base;
    nfs::u32 size;
};
Region regionOf(nfs::u32 address);

/* What the machine is said to have: 256 MB of memory, all of it free, and 2 GB of addresses. */
constexpr nfs::u32 kPhysicalBytes = 0x10000000;
constexpr nfs::u32 kAddressBytes = 0x7fffffff;

}  // namespace memory

/* ---- Text ---- */

namespace text
{

/* The port's text is one byte a character, no lead bytes: the code page it reports. */
constexpr nfs::u32 kCodePage = 850;

/* The string `text` in capitals, in place. */
void upper(char* text);

/* The `length` bytes at `text` in capitals, in place. */
void upper(char* text, nfs::u32 length);

/* Up to `count` bytes of `from` made wide characters into `to` (none written for a null `to`):
 * a byte-order mark (0xfffe) first, then a character for each byte, up to and with the first
 * zero byte of `from`.  How many bytes it took. */
int widen(const char* from, int count, nfs::u16* to);

}  // namespace text

/* ---- Messages ---- */

/* A message box's look. */
enum class Box
{
    plain,
    error,
    warning,
};

/* The box `text` under `caption` shown, waited for: it has one button and no answer. */
void messageBox(const char* text, const char* caption, Box look);

/* `text` to the debugger: the log. */
void debugText(const char* text);

/* ---- Settings: what the game reads from the registry ---- */

namespace settings
{

/* The keys the settings are under. */
enum class Root
{
    classes,
    currentUser,
    localMachine,
};

/* A setting: a number or a text. */
struct Setting
{
    bool number = false;
    nfs::u32 value = 0;
    std::string text;
};

/* The setting `name` under `key` of `root` (its parts split by \, each found whatever its case);
 * nullopt when the key or the setting is not there. */
std::optional<Setting> find(App* app, Root root, const char* key, const char* name);

/* Whether the key `key` of `root` is there. */
bool has(App* app, Root root, const char* key);

/* The setting `setting` to the `room` bytes at `to` in the game's memory, as the registry hands
 * one: a number's four bytes or a text with its ending, written only if it fits.  The bytes it
 * takes. */
nfs::u32 copyTo(App* app, const Setting& setting, nfs::u32 to, nfs::u32 room);

}  // namespace settings

/* ---- The executable's version ---- */

/* The FileVersion string of the game's executable (its resources are not in the game's memory,
 * so the port names it): the version a network race compares between machines.  A joining
 * machine waits until it has one that is not empty. */
void setFileVersion(const char* value);
const std::string& fileVersion();

/* The value the game's version block has under `key`: the FileVersion string, for a key that
 * names it (any case) once it is set; nullopt for any other. */
std::optional<std::string> versionValue(const char* key);

/* The version block's size, the game's copy of it: the version string at kVersionText in it. */
constexpr nfs::u32 kVersionBytes = 64;
constexpr nfs::u32 kVersionText = 4;

}  // namespace os
}  // namespace nfs3hp

#endif
