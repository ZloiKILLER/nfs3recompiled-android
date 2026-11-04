/* The decompiled screens -- src/nfs3hp/native_frontend_options.cpp, _cars.cpp,
 * _tracks.cpp and _main.cpp -- against the generated code they stand in for, on the
 * desktop, without the game or its data.
 *
 * The generated functions are copied out of src/nfs3hp/disassembly without
 * the port's hooks (extract.py) and built with the port's own headers.  Every
 * function they call outside these screens is a stand-in here (mock):
 * it records the call -- where, the registers and stack arguments the real one
 * reads, and a hash of the menus' and settings' memory as it stands at the
 * call -- and answers from a script, the same script for both runs.  It
 * clobbers the registers the real one does not keep, with values of its own,
 * so a caller counting on one the real function changes shows up.  The few
 * calls whose effect the screens read back -- a copy, a fill, sprintf, a
 * slider's value on the x87, the list of screen modes -- do it, and those
 * handed a string record what it says.
 *
 * Each case starts both from the same memory, registers and x87 (its control
 * word in each precision): the generated code, then the decompiled one; then
 * all of memory but the stack below the returned esp, the registers, the x87's
 * status word and depth, and the two runs' calls are compared.
 *
 * The data is made up: menus and items, the settings each screen reads, with
 * values on both sides of every comparison the screens make.
 *
 * See run.sh. */
#include <algorithm>
#include <nfs3hp.h>
#include <native_frontend.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <sys/mman.h>

namespace x86
{
void assertLog(const char* file, int line, const char* expr)
{
    std::printf("assert %s:%d %s\n", file, line, expr);
}
}

namespace
{

constexpr x86::reg32 kMemory = 0x1000000;
constexpr x86::reg32 kStack = 0xff0000;

x86::reg8* g_memory = nullptr;
std::map<x86::reg32, win32::MethodPtr> g_methods;

x86::reg32 word32(x86::reg32 at)
{
    x86::reg32 value;
    std::memcpy(&value, g_memory + at, 4);
    return value;
}

void setWord32(x86::reg32 at, x86::reg32 value)
{
    std::memcpy(g_memory + at, &value, 4);
}

void setFloat(x86::reg32 at, float value)
{
    std::memcpy(g_memory + at, &value, 4);
}

/* The script both runs answer from, and what each run recorded. */
std::mt19937 g_script;
std::vector<x86::reg32> g_log;
unsigned g_calls = 0;
x86::reg32 g_caseAddress = 0;  // the function a case is of

x86::reg32 scripted(x86::reg32 below)
{
    return below ? x86::reg32(g_script() % below) : x86::reg32(g_script());
}

/* The memory the screens read and write, hashed at each call. */
/* The group these cases are of (windows/run.py): its world, entries and memory. */
const std::string g_group = std::getenv("FRONTEND_CHECKS_GROUP") ? std::getenv("FRONTEND_CHECKS_GROUP") : "";

const std::pair<x86::reg32, x86::reg32> kWatched[] = {
    { 0x550000, 0x570000 }, { 0x5f3000, 0x5f4000 }, { 0x604000, 0x608000 },
    { 0x660000, 0x664000 }, { 0x6fb000, 0x6fe000 }, { 0x7a2000, 0x7a4000 },
    { 0x5e1000, 0x5e2000 }, { 0x677000, 0x678000 }, { 0x7dd000, 0x7de000 }, { 0x8b8000, 0x8bb000 },
    { 0x678000, 0x679000 }, { 0x5f4000, 0x5f5000 }, { 0x664000, 0x665000 },
};

using Ranges = std::vector<std::pair<x86::reg32, x86::reg32>>;

/* kWatched and this group's own ranges, sorted and merged. */
const Ranges& watchedRanges()
{
    static const Ranges ranges = [] {
        Ranges all(std::begin(kWatched), std::end(kWatched));
#include "group_watched.inc"
        std::sort(all.begin(), all.end());
        Ranges merged;
        for (const auto& range : all)
        {
            if (!merged.empty() && range.first <= merged.back().second)
                merged.back().second = std::max(merged.back().second, range.second);
            else
                merged.push_back(range);
        }
        return merged;
    }();
    return ranges;
}

/* What the hash reads: all of it but the rows the items stand in, which mainsplt
 * moves as the port has it (checked by compare()). */
const Ranges& hashedRanges()
{
    static constexpr x86::reg32 kRowsFrom = 0x559c68, kRowsTo = 0x559cb8;
    static const Ranges ranges = [&] {
        Ranges out;
        for (const auto& range : watchedRanges())
        {
            if (range.first < kRowsFrom && kRowsTo < range.second)
            {
                out.push_back({ range.first, x86::reg32(kRowsFrom) });
                out.push_back({ x86::reg32(kRowsTo), range.second });
            }
            else
                out.push_back(range);
        }
        return out;
    }();
    return ranges;
}

/* Every call out records this, so that a write made on the wrong side of a call
 * shows: most of a case's time, hence four lanes, a word of 32 bytes at a time. */
x86::reg32 watchedHash()
{
    constexpr std::uint64_t kPrime = 1099511628211ull;
    std::uint64_t h[4] = { 1469598103934665603ull, 0x9e3779b97f4a7c15ull, 0xc2b2ae3d27d4eb4full, 0x165667b19e3779f9ull };
    for (const auto& range : hashedRanges())
    {
        const x86::reg8* at = g_memory + range.first;
        const x86::reg8* const end = g_memory + range.second;
        for (; at + 32 <= end; at += 32)
        {
            std::uint64_t v[4];
            std::memcpy(v, at, 32);
            h[0] = (h[0] ^ v[0]) * kPrime;
            h[1] = (h[1] ^ v[1]) * kPrime;
            h[2] = (h[2] ^ v[2]) * kPrime;
            h[3] = (h[3] ^ v[3]) * kPrime;
        }
        for (; at + 8 <= end; at += 8)
        {
            std::uint64_t v;
            std::memcpy(&v, at, 8);
            h[0] = (h[0] ^ v) * kPrime;
        }
    }
    const std::uint64_t all = (h[0] ^ (h[1] >> 13)) * kPrime ^ (h[2] ^ (h[3] >> 29)) * kPrime;
    return x86::reg32(all ^ (all >> 32));
}

enum Reg : std::uint8_t
{
    EAX = 1,
    EBX = 2,
    ECX = 4,
    EDX = 8,
    ESI = 16,
    EDI = 32,
};

enum class Kind
{
    Plain,         // a value from the script
    Bool,          // 0 or 1
    FindItem,      // 0 or an item
    KeyHandler,    // a widget's key handler: it may move the keys and the controller chosen
    SliderValue,   // a float on the x87
    Copy,          // eax to edx, ebx bytes
    Fill,          // edx zero bytes at eax
    Sprintf,
    ListModes,     // the count of modes at 0x559218
    CurrentMode,   // the chosen mode's record to esi
    ControlsOf,
    Focus,         // 0, 1 or 2
    NoEax,         // leaves eax as it is
    Shape,         // a shape's record: memory to read
    Tuning,        // 0 or a player's tuning record, 0x1c bytes
    EditText,      // the dialog's line typed in: a string of the script's
    Memset,        // memset: eax, the byte in edx, ebx of it
    Strncpy,       // strncpy: edx to eax, ebx at most
};

struct Spec
{
    std::uint8_t in;       // the registers it reads
    std::uint8_t stack;    // the stack arguments it reads
    std::uint8_t pops;     // what its ret pops
    std::uint8_t clobbers; // the registers it changes besides
    Kind kind;
    bool string = false;   // eax a string: what it says is recorded
};

/* An item's handler as its screen shows it: no code of the game's, a stand-in. */
constexpr x86::reg32 kShowHandler = 0x7e0100;

const std::map<x86::reg32, Spec> kSpecs = {
    // What sub_439ce0 calls: it runs as itself here, by name, from sub_4270c0.
    { 0x438ad0, { EAX | EDX, 0, 0, 0, Kind::Plain } },
    { 0x439990, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x442a40, { EAX | EDX, 0, 0, EDX, Kind::FindItem } },
    { 0x4429c0, { EAX | EDX, 0, 0, EDX, Kind::Bool } },
    { 0x461270, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::KeyHandler } },
    { 0x4624a0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::KeyHandler } },
    { 0x463ff0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::KeyHandler } },
    { 0x46c950, { EAX, 2, 8, 0, Kind::Plain } },
    { 0x46c9b0, { EAX | EDX, 0, 0, EDX, Kind::SliderValue } },
    { 0x46cac0, { EAX | EDX, 0, 0, 0, Kind::NoEax } },
    { 0x46cae0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x46cca0, { EAX | EBX | EDX, 0, 0, 0, Kind::NoEax } },
    { 0x451530, { 0, 1, 4, 0, Kind::Plain } },
    { 0x4ea4f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Copy } },
    { 0x4e070c, { EAX | EDX, 0, 0, EDX, Kind::Fill } },
    { 0x435b40, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
    { 0x4358f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x435df0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4482c0, { 0, 0, 0, 0, Kind::ListModes } },
    { 0x448330, { ESI, 0, 0, ESI, Kind::CurrentMode } },
    { 0x4904d0, { EAX, 0, 0, 0, Kind::ControlsOf } },
    { 0x4904f0, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x44ac90, { EAX, 0, 0, 0, Kind::Plain, true } },
    { 0x4d5db0, { EAX, 0, 0, 0, Kind::Plain, true } },
    { 0x4df690, { 0, 0, 0, 0, Kind::Sprintf } },
    { 0x43c040, { 0, 0, 0, 0, Kind::Plain } },
    { 0x44a660, { EAX, 0, 0, 0, Kind::Bool } },
    { 0x4d1850, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x4446c0, { 0, 0, 0, 0, Kind::Focus } },
    { 0x4446d0, { EAX | EBX | ECX | EDX, 4, 16, EBX | ECX | EDX, Kind::Plain } },
    { 0x444be0, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x445f00, { 0, 0, 0, 0, Kind::NoEax } },
    { 0x445f10, { 0, 0, 0, 0, Kind::Plain } },
    { 0x445f50, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x4438b0, { 0, 0, 0, 0, Kind::Bool } },
    { 0x44aeb0, { 0, 0, 0, 0, Kind::Plain } },
    { 0x44a4f0, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x4d9120, { 0, 0, 0, EBX | EDI | ESI, Kind::Plain } },
    { 0x4d9140, { 0, 0, 0, EBX | EDI | ESI, Kind::Plain } },
    { 0x4109d0, { 0, 0, 0, EBX, Kind::Plain } },
    { 0x410aa0, { 0, 0, 0, EBX | EDI | ESI, Kind::Plain } },
    { 0x410b20, { 0, 0, 0, EDI, Kind::Plain } },
    { 0x40feb0, { 0, 0, 0, EBX | EDI | EDX | ESI, Kind::Plain } },
    { 0x40fcf0, { EAX | EDX, 0, 0, 0, Kind::Plain } },
    { 0x4136e0, { 0, 0, 0, 0, Kind::Plain } },
    { 0x410330, { 0, 0, 0, 0, Kind::Plain } },
    { 0x4103a0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4107c0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
    { 0x4e7470, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4e9420, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
    { 0x4e95fc, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4182b0, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
    { 0x40faf0, { 0, 0, 0, EBX | EDI | ESI, Kind::Bool } },
    // The cars' and the tracks' screens'.
    { 0x439520, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x439660, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x4eb1d0, { EAX, 0, 0, EBX | EDI | ESI, Kind::Plain, true } },
    { 0x4ead20, { 0, 0, 0, 0, Kind::Plain } },
    { 0x4eb160, { EAX, 0, 0, 0, Kind::Plain, true } },
    { 0x4ead90, { 0, 0, 0, 0, Kind::Plain } },
    { 0x439760, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x439790, { EAX | EDX, 0, 0, 0, Kind::Plain } },
    { 0x4397d0, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x456a20, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x4530c0, { EAX, 0, 0, EDI, Kind::Bool } },
    { 0x4396b0, { EAX, 0, 0, 0, Kind::Bool } },
    { 0x439ce0, { 0, 0, 0, 0, Kind::Bool } },
    { 0x439a50, { EAX, 0, 0, 0, Kind::Bool } },
    { 0x4dc550, { EAX | EDX, 0, 0, EDX, Kind::Tuning } },
    { 0x4dca50, { EAX, 0, 0, 0, Kind::Bool } },
    { 0x4104f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
    { 0x4d6390, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4ef270, { EAX | EDX, 0, 0, EDX, Kind::Shape, false } },
    { 0x4ef1e0, { EAX | EDX, 0, 0, EDX, Kind::Shape } },  // edi it saves round the one path that uses it
    { 0x456800, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4c9240, { EAX, 0, 0, 0, Kind::Bool, true } },
    { 0x4c95f0, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x4c9490, { 0, 0, 0, ESI, Kind::Plain } },
    { 0x4e8c1c, { EAX, 0, 0, EDI, Kind::Plain } },
    { 0x4e1890, { EAX, 0, 0, 0, Kind::Plain } },
    // The main menu's.
    { 0x452020, { EAX | EDX, 0, 0, EDX, Kind::Plain, true } },
    { 0x444c10, { 0, 0, 0, 0, Kind::EditText } },
    { 0x444c20, { EAX | EBX | ECX | EDX, 5, 20, EBX | ECX | EDX, Kind::Plain } },
    { 0x4721f0, { EAX, 0, 0, 0, Kind::Bool, true } },
    { 0x496200, { EAX, 0, 0, 0, Kind::Bool } },
    { 0x4a65e0, { 0, 0, 0, 0, Kind::Plain } },
    // The menus' text past the language's lines: names (native_frontend_text.cpp).
    { 0x4d17b0, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x43d9f0, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x448400, { EAX, 0, 0, 0, Kind::Plain } },
    // What the main menu's dialogs and the menu loop's parts call: the font,
    // the dialogs' layout and pointer, the screens' art, an item's handler.
    { 0x452100, { EAX, 0, 0, 0, Kind::NoEax } },
    { 0x443a90, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
    { 0x443c00, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
    { 0x443d20, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
    { 0x444570, { EAX, 0, 0, 0, Kind::Plain } },
    { 0x444a80, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
    { 0x49bfc0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
    { 0x443860, { EAX | EDX, 0, 0, 0, Kind::Bool, true } },
    { 0x4e0640, { EAX | EBX | EDX, 0, 0, 0, Kind::Memset } },
    { 0x4e0e30, { EAX | EBX | EDX, 0, 0, 0, Kind::Strncpy } },
    { 0x440970, { 0, 0, 0, 0, Kind::Plain } },
    { kShowHandler, { EAX | EDX, 0, 0, 0, Kind::Plain } },
};

x86::reg32* reg(x86::CPU& cpu, std::uint8_t r)
{
    switch (r)
    {
    case EAX: return &cpu.eax;
    case EBX: return &cpu.ebx;
    case ECX: return &cpu.ecx;
    case EDX: return &cpu.edx;
    case ESI: return &cpu.esi;
    default: return &cpu.edi;
    }
}

// The rows the screens' items stand in (native_frontend_main.cpp): x and y,
// int16 every four bytes.
constexpr x86::reg32 kRowX = 0x559c6c, kRowY = 0x559c94;

// Where the dialog's typed line is, for the main menu's name dialogs.
constexpr x86::reg32 kEditText = 0x5f3fc0;

// The items findItem hands out, and the screens' slider slots that may hold them.
constexpr x86::reg32 kItems = 0x604ee0;
constexpr x86::reg32 kItemSize = 0xbc;
constexpr std::uint32_t kPoolFirst = 10;
constexpr std::uint32_t kPoolItems = 30;

x86::reg32 poolItem(std::uint32_t i)
{
    return kItems + kItemSize * (kPoolFirst + i);
}

/* Every slider at this value for a case, when not NaN: the tuning screen
 * then finds none moved. */
float g_fixedSlider = std::numeric_limits<float>::quiet_NaN();

const float kValues[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 0.1f, 0.9f, 0.33f, 0.6f, 0.05f };

float someValue(std::mt19937& random)
{
    // Often the same few, so that the ends the deadzone screen kept match.
    if (random() % 3 == 0)
        return random() % 2 ? 0.5f : 0.25f;
    const unsigned pick = random() % 14;
    if (pick < std::size(kValues))
        return kValues[pick];
    if (pick == 10)
        return -0.25f;
    if (pick == 11)
        return 1.3f;
    return float(random() % 100000) / 100000.0f;
}

/* A string of guest memory, at most `limit` characters. */
std::string guestString(x86::reg32 at, std::size_t limit = 200)
{
    std::string text;
    while (text.size() < limit && at < kMemory && g_memory[at] != 0)
        text += char(g_memory[at++]);
    return text;
}

/* sprintf, Watcom's: its format's %s, %d, %x and %c, flags and width, the
 * arguments from the stack, each one recorded. */
void sprintfMock(x86::CPU& cpu)
{
    const x86::reg32 buffer = word32(cpu.esp + 4);
    const x86::reg32 format = word32(cpu.esp + 8);
    const std::string fmt = guestString(format);
    g_log.push_back(buffer);
    g_log.push_back(format);
    std::string out;
    x86::reg32 argument = cpu.esp + 12;
    for (std::size_t i = 0; i < fmt.size(); ++i)
    {
        if (fmt[i] != '%' || i + 1 == fmt.size())
        {
            out += fmt[i];
            continue;
        }
        std::size_t j = i + 1;
        while (j < fmt.size() && std::strchr("-+ #0123456789", fmt[j]))
            ++j;
        if (j == fmt.size())
            break;
        const std::string spec = "%" + fmt.substr(i + 1, j - i - 1) + fmt[j];
        char piece[256] = "";
        const x86::reg32 value = word32(argument);
        switch (fmt[j])
        {
        case 's':
            g_log.push_back(value);
            std::snprintf(piece, sizeof piece, spec.c_str(), guestString(value).c_str());
            argument += 4;
            break;
        case 'd':
        case 'x':
        case 'X':
        case 'c':
            g_log.push_back(value);
            std::snprintf(piece, sizeof piece, spec.c_str(), int(value));
            argument += 4;
            break;
        case '%':
            piece[0] = '%';
            break;
        default:
            std::snprintf(piece, sizeof piece, "<%c>", fmt[j]);
            break;
        }
        out += piece;
        i = j;
    }
    if (out.size() > 255)
        out.resize(255);
    std::memcpy(g_memory + buffer, out.c_str(), out.size() + 1);
    cpu.eax = x86::reg32(out.size());
}

}

namespace nfs3hp
{

/* The dialogs' NFS emblem, drawn wide-screen (tools/apply_widescreen.py):
 * nothing here. */
void dialogEmblem(bool) {}

/* Whether the Graphics menu's mode is Dynamic Fullscreen (nfs3hp_main.cpp's,
 * by the driver's table): none of the modes the cases make up is. */
bool dynamicFullscreenMode(x86::reg32) { return false; }

/* What the port's patches in the groups' functions call: their stand-ins. */
#include "group_hooks.inc"

/* A race's depth on the Screen Size list, and Dynamic Fullscreen's line: recorded. */
x86::reg32 shownDepth(x86::reg32 depth)
{
    g_log.push_back(0x7e480000 | depth);
    return depth == 16 ? 32 : depth;
}

void dynamicFullscreenLabel(win32::WinApplication*, x86::reg32 entry)
{
    g_log.push_back(0x7e490000);
    g_log.push_back(entry);
}

/* The port the race's transport is on, as the original has it. */
x86::reg32 networkPort() { return 0x406; }

/* The port's keyboard, told the game takes typing: recorded. */
void textEntry(bool taking)
{
    g_log.push_back(0x7e470000 | (taking ? 1 : 0));
}

bool hasStandIn(x86::reg32 address)
{
    static const x86::reg32 kGroups[] = {
#include "group_addresses.inc"
    };
    /* This process's own group's run as themselves; another group's are its to check. */
    static const std::vector<x86::reg32> own = [] {
        std::vector<x86::reg32> list;
#include "group_own.inc"
        return list;
    }();
    return std::find(std::begin(kGroups), std::end(kGroups), address) != std::end(kGroups)
        && std::find(own.begin(), own.end(), address) == own.end() && kSpecs.count(address) != 0;
}

/* A stand-in for a game function: the call recorded, answered from the
 * script. */
void mock(win32::WinApplication*, x86::CPU& cpu, x86::reg32 address)
{
    auto found = kSpecs.find(address);
    if (found == kSpecs.end())
    {
        std::printf("no stand-in for sub_%06x\n", unsigned(address));
        std::fflush(stdout);
        std::abort();
    }
    const Spec& spec = found->second;
    if (++g_calls > 200000)
    {
        // A case that does not end (a world the game never has): said, not waited for.
        std::printf("FAIL sub_%06x: a case has not ended after %u calls, the last sub_%06x\n", unsigned(g_caseAddress), unsigned(g_calls), unsigned(address));
        std::fflush(stdout);
        std::_Exit(3);
    }
    g_log.push_back(0xca110000);
    g_log.push_back(address);
    for (const std::uint8_t r : { EAX, EBX, ECX, EDX, ESI, EDI })
        if (spec.in & r)
            g_log.push_back(*reg(cpu, r));
    for (unsigned i = 0; i < spec.stack; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    if (spec.string)
    {
        for (const char c : guestString(cpu.eax, 64))
            g_log.push_back(x86::reg32(std::uint8_t(c)));
    }
    g_log.push_back(watchedHash());
    g_log.push_back(cpu.fpu.count);

    x86::reg32 result = scripted(0);
    switch (spec.kind)
    {
    case Kind::Plain:
        break;
    case Kind::NoEax:
        result = cpu.eax;
        break;
    case Kind::Bool:
        result = scripted(3) != 0 ? 1 : 0;
        break;
    case Kind::Focus:
        result = scripted(3);
        break;
    case Kind::FindItem:
        result = scripted(5) == 0 ? 0 : poolItem(scripted(kPoolItems));
        break;
    case Kind::KeyHandler:
        // The feeds' keys, and the controller chosen, as a list or button moves them.
        if (scripted(4) != 0)
        {
            setWord32(0x6fbc04 + 4 * scripted(3), scripted(10));
            setWord32(0x558a74, scripted(4));
            // The car a tuned player's list chose.
            const x86::reg32 player = word32(0x55304c);
            if (player == 0x6fd2bc || player == 0x6fd328)
                setWord32(player, scripted(3) == 0 ? word32(player) : scripted(40));
        }
        result = scripted(12);
        break;
    case Kind::SliderValue:
        cpu.fpu.push(x86::Float(g_fixedSlider == g_fixedSlider ? g_fixedSlider : someValue(g_script)));
        result = cpu.eax;
        break;
    case Kind::Copy:
    {
        const x86::reg32 bytes = cpu.ebx < 0x1000 ? cpu.ebx : 0x1000;
        g_log.push_back(bytes);
        std::memmove(g_memory + cpu.edx, g_memory + cpu.eax, bytes);
        break;
    }
    case Kind::Fill:
        std::memset(g_memory + cpu.eax, 0, cpu.edx < 0x1000 ? cpu.edx : 0x1000);
        break;
    case Kind::Sprintf:
        sprintfMock(cpu);
        result = cpu.eax;
        break;
    case Kind::ListModes:
        setWord32(0x559218, 1 + scripted(8));
        break;
    case Kind::CurrentMode:
    {
        const x86::reg32 index = word32(0x6fbc14) % 16;
        std::memcpy(g_memory + cpu.esi, g_memory + 0x661220 + 0x114 * index, 0x114);
        result = cpu.esi;
        break;
    }
    case Kind::ControlsOf:
        result = cpu.eax == 1 ? 0x6fd240 : 0x6fd20c;
        break;
    case Kind::Shape:
        result = 0x5f3c00 + 16 * scripted(16);
        break;
    case Kind::Tuning:
        result = scripted(3) == 0 ? 0 : 0x5f3e00 + 4 * scripted(8);
        break;
    case Kind::Memset:
        std::memset(g_memory + cpu.eax, int(cpu.edx & 0xff), cpu.ebx < 0x1000 ? cpu.ebx : 0x1000);
        result = cpu.eax;
        break;
    case Kind::Strncpy:
    {
        // As the runtime's: up to ebx, the rest of them 0 past the source's end.
        const x86::reg32 most = cpu.ebx < 0x1000 ? cpu.ebx : 0x1000;
        bool ended = false;
        for (x86::reg32 i = 0; i < most; ++i)
        {
            const x86::reg8 c = ended ? 0 : g_memory[cpu.edx + i];
            ended = ended || c == 0;
            g_memory[cpu.eax + i] = c;
        }
        result = cpu.eax;
        break;
    }
    case Kind::EditText:
    {
        // A name of 0 to 14 letters, past the 10 a player's may have now and then.
        const x86::reg32 length = scripted(15);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[kEditText + i] = x86::reg8('A' + scripted(26));
        g_memory[kEditText + length] = 0;
        result = kEditText;
        break;
    }
    }
    for (const std::uint8_t r : { EBX, ECX, EDX, ESI, EDI })
        if (spec.clobbers & r)
            *reg(cpu, r) = 0xc1000000 | (g_calls << 8) | r;
    cpu.eax = result;
    cpu.esp += 4 + spec.pops;
}

/* The game's own natives (native_game.cpp) the generated code calls by
 * name: the stand-ins they were at their addresses. */
void Is3DSoundAvailable(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x40faf0);
}

/* The menus' text, native whole (native_frontend_text.cpp, which
 * tools/game_checks checks): the stand-ins they were at their addresses. */
namespace frontend
{

void languageText(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4d1850);
}

void specialText(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4d17b0);
}

void nameValid(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4721f0);
}

void controllerName(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x43d9f0);
}

void displayModeName(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x448400);
}

}

/* The Watcom runtime's functions the generated code calls by name, native in
 * the port (native_watcom.cpp): __CHP as it is -- st(0) rounded toward zero,
 * the screens read what it gives -- and sprintf and mkdir as the stand-ins
 * they were at their addresses. */
namespace watcom
{

void CHP(win32::WinApplication*, x86::CPU& cpu)
{
    const auto rc = cpu.fpu.control.rc;
    cpu.fpu.control.rc = 3;
    cpu.fpu.st(0) = cpu.fpu.rndint(cpu.fpu.st(0));
    cpu.fpu.control.rc = rc;
    cpu.esp += 4;
}

void sprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4df690);
}

void mkdir(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4eb1d0);
}

}

}

/* The menus' functions native whole that the other functions tested call
 * (extract.py's STAND_IN_ELSEWHERE): stood in for but in their own cases. */
const x86::reg32 kStandInElsewhere[] = { 0x442a40, 0x44a660, 0x4446c0 };
x86::reg32 g_testing = 0;

namespace win32
{

WinApplication::WinApplication(const char*, x86::reg32, const std::vector<Section>&)
{
#ifdef _WIN32
    // Watched for writes, for compare() to copy and compare only what changed.
    void* memory = VirtualAlloc(nullptr, kMemory, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE);
    if (!memory)
        std::abort();
#else
    void* memory = mmap(nullptr, kMemory, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
        std::abort();
#endif
    m_memory = static_cast<x86::reg8*>(memory);
    g_memory = m_memory;
}

WinApplication::~WinApplication() {}

int WinApplication::s_traceApi = 0;
bool WinApplication::readTraceApi() { return false; }
void WinApplication::traceCall(const Method&, x86::reg32) {}
void WinApplication::yieldContext(const x86::CPU&) {}

void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    auto method = g_methods.find(address);
    const bool elsewhere = address != g_testing
        && (std::find(std::begin(kStandInElsewhere), std::end(kStandInElsewhere), address) != std::end(kStandInElsewhere)
            || nfs3hp::hasStandIn(address));
    if (method == g_methods.end() || elsewhere)
    {
        nfs3hp::mock(this, cpu, address);
        return;
    }
    method->second(this, cpu);
}

void MemMap::usedRanges(std::vector<std::pair<x86::reg32, x86::reg32>>& ranges)
{
    ranges.assign(1, {0, kMemory});
}

}

namespace nfs3hp
{

#include "build/mocks.inc"

Application::Application(const char* appName)
    : WinApplication(appName, 0x400000, {})
{
    g_methods = {
#include "build/mocks.inc.generated"
        {0x4dfd56, &watcom::CHP},
    };
}

}

namespace
{

typedef bool (*Native)(win32::WinApplication*, x86::CPU&);

/* A function native whole, which has no generated code to step aside to. */
template <void (*Function)(win32::WinApplication*, x86::CPU&)>
bool whole(win32::WinApplication* app, x86::CPU& cpu)
{
    Function(app, cpu);
    return true;
}

std::mt19937 g_random(1998);

x86::reg32 random(x86::reg32 below)
{
    return below ? x86::reg32(g_random() % below) : x86::reg32(g_random());
}

/* The exe's data, its strings among it: the decompiled code checks them. */
void loadImage(const char* path)
{
    std::ifstream file(path, std::ios::binary);
    std::vector<char> d((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (d.size() < 0x400)
    {
        std::printf("cannot read %s\n", path);
        std::exit(1);
    }
    auto u32 = [&](std::size_t at) { x86::reg32 v; std::memcpy(&v, &d[at], 4); return v; };
    auto u16 = [&](std::size_t at) { std::uint16_t v; std::memcpy(&v, &d[at], 2); return v; };
    const x86::reg32 pe = u32(0x3c);
    const unsigned sections = u16(pe + 6);
    const unsigned optional = u16(pe + 20);
    const x86::reg32 base = u32(pe + 24 + 28);
    for (unsigned i = 0; i < sections; ++i)
    {
        const std::size_t s = pe + 24 + optional + 40 * i;
        const x86::reg32 va = u32(s + 12), at = u32(s + 20);
        x86::reg32 raw = u32(s + 16);
        if (at >= d.size())
            continue;
        if (raw > d.size() - at)
            raw = x86::reg32(d.size() - at);
        if (raw && base + va + raw < kStack - 0x100000)
            std::memcpy(g_memory + base + va, &d[at], raw);
    }
}

std::vector<x86::reg8> g_image;

void fillRandom(x86::reg32 from, x86::reg32 to)
{
    std::uint64_t state = (std::uint64_t(g_random()) << 32) | g_random();
    for (x86::reg32 at = from; at < to; at += 8)
    {
        // splitmix64: the mt19937 a byte at a time was a good part of a case.
        state += 0x9e3779b97f4a7c15ull;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        z ^= z >> 31;
        std::memcpy(g_memory + at, &z, std::min<x86::reg32>(8, to - at));
    }
}

x86::reg32 maybeItem(unsigned zeroIn)
{
    return random(zeroIn) == 0 ? 0 : poolItem(random(kPoolItems));
}

x86::reg32 g_menu = 0;

/* A world for one case: the image, then the screens' memory made up. */
void makeWorld()
{
    // The image, only the pages of it that a case changed: writing them all would
    // leave them all to copy and compare.
    for (std::size_t at = 0; at < g_image.size(); at += 0x1000)
    {
        const std::size_t length = std::min<std::size_t>(0x1000, g_image.size() - at);
        if (std::memcmp(g_memory + 0x534000 + at, g_image.data() + at, length) != 0)
            std::memcpy(g_memory + 0x534000 + at, g_image.data() + at, length);
    }
    for (const auto& range : watchedRanges())
        fillRandom(range.first, range.second);
    fillRandom(kStack - 0x8000, kStack + 0x100);

    // The menu: its first item and the one selected, in the pool.
    g_menu = 0x6040d0 + 0x24 * 3;
    g_memory[g_menu + 0x18] = x86::reg8(kPoolFirst + random(10));
    g_memory[g_menu + 0x19] = 0;
    g_memory[g_menu + 0x1a] = x86::reg8(random(10));
    g_memory[g_menu + 0x1b] = 0;
    for (std::uint32_t i = 0; i < kPoolItems + 10; ++i)
    {
        const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
        setWord32(item, random(3) ? 5 : random(20));
    }

    setWord32(0x559234, random(2));                    // in a race
    setWord32(0x6fd3b0, random(3));                    // players
    setWord32(0x6fbb54, random(3) == 0);
    for (x86::reg32 at : { 0x6fbc04u, 0x6fbc08u, 0x6fbc0cu, 0x5f3910u, 0x5f3914u, 0x5f3918u })
        setWord32(at, random(5) == 0 ? x86::reg32(random(14)) - 2 : random(10));
    for (x86::reg32 at = 0x6fbbd0; at < 0x6fbc04; at += 4)
        setWord32(at, random(4) == 0 ? random(3) : random(128));
    for (x86::reg32 at : { 0x552920u, 0x552924u, 0x5f3960u, 0x5f396cu })
        setWord32(at, random(128));
    setWord32(0x6fbbe4, random(5));
    setWord32(0x5f3968, random(2) ? word32(0x6fbbe4) : random(5));
    setWord32(0x6fbc00, random(2));
    setWord32(0x5f3970, random(2) ? word32(0x6fbc00) : random(2));
    setWord32(0x5f3964, random(2) ? word32(0x6fbbec) : random(128));
    for (x86::reg32 at = 0x554f20; at <= 0x554f34; at += 4)
        setWord32(at, random(4) == 0);
    setWord32(0x554f38, random(2) ? 0 : random(0));
    for (x86::reg32 at = 0x554f3c; at <= 0x554f4c; at += 4)
        setWord32(at, maybeItem(3));
    g_memory[0x552909] = x86::reg8(random(2));
    g_memory[0x55e890] = x86::reg8(random(2));

    // Controllers.
    setWord32(0x6fd204, random(6) == 0 ? x86::reg32(-1) : random(5));
    for (x86::reg32 c = 0; c < 6; ++c)
    {
        const x86::reg32 record = 0x6fc970 + 0x88 * c;
        setWord32(record, random(3) == 0 ? 0 : random(0));
        setWord32(record + 0x40, random(0));
        for (x86::reg32 k = 0; k < 16; ++k)
            setWord32(record + 0x48 + 4 * k, random(4) == 0 ? random(0) : random(2) ? 25 * random(5) : random(101));
    }
    setWord32(0x558a70, random(4));
    setWord32(0x558a74, random(2) ? word32(0x558a70) : random(4));
    for (x86::reg32 k = 0; k < 8; ++k)
    {
        setWord32(0x558a10 + 4 * k, maybeItem(3));
        setFloat(0x558a30 + 8 * k, someValue(g_random));
        setFloat(0x558a34 + 8 * k, someValue(g_random));
        setWord32(0x558ac8 + 4 * k, random(4) == 0 ? random(0) : random(65536));
    }
    setWord32(0x660fc0, random(2));
    setWord32(0x5f3da4, random(2));

    // Force feedback.
    for (x86::reg32 at = 0x6611e0; at <= 0x6611f4; at += 4)
        setWord32(at, maybeItem(4));
    for (x86::reg32 at = 0x6fd1f0; at <= 0x6fd200; at += 4)
        setWord32(at, random(4) == 0 ? random(0) : random(101));
    setWord32(0x6fc9b4, random(4) == 0 ? random(0) : random(101));
    setWord32(0x661210, random(3));

    // Graphics.
    setWord32(0x6fbc14, random(12));
    setWord32(0x559218, random(12));
    setWord32(0x6fbb4c, random(8));
    setWord32(0x7a3a68, random(2));
    setWord32(0x6fbc34, random(2));
    setFloat(0x6fbc38, someValue(g_random));
    for (x86::reg32 m = 0; m < 16; ++m)
        setWord32(0x661220 + 0x114 * m + 0x10, random(2));

    std::memcpy(g_memory + 0x7a2db4, "fedata\\art\\", 12);

    // The cars' screens.
    // The kind of race: 0 to 3, now and then past them -- by a few only, for
    // mainmult indexes its list with it, unchecked.
    setWord32(0x6fd3b8, random(5) == 0 ? 4 + random(12) : random(4));
    setWord32(0x6fd2bc, random(40));
    setWord32(0x6fd328, random(40));
    static const x86::reg32 kPlayers[] = { 0x6fd2bc, 0x6fd328, 0x6fd2bc, 0x6fd328, 0x5f3e80 };
    setWord32(0x55304c, kPlayers[random(std::size(kPlayers))]);
    setWord32(0x553048, random(6));
    setWord32(0x553044, random(5));
    setWord32(0x6fd4d4, random(9));
    setWord32(0x6fd4d0, random(4));
    setWord32(0x553040, random(4));
    setWord32(0x552594, random(2));
    setWord32(0x6fbbec, random(3) == 0 ? 6 : random(8));
    // The developer's recorder.
    g_memory[0x55eb2f] = x86::reg8(random(2) ? 0x40 : random(256));
    setWord32(0x553038, random(10));
    g_memory[0x564850] = x86::reg8(random(3) != 0);
    g_memory[0x564831] = x86::reg8(random(3) != 0);
    setWord32(0x5e10d0, random(3) == 0 ? random(0) : 25 + random(9));
    setFloat(0x55302c, float(random(64)) / 32.0f);
    // The tuning: each setting's least, most and default, the edited and the
    // shown ones, often the sliders' values.
    for (x86::reg32 i = 0; i < 7; ++i)
    {
        const float least = random(4) == 0 ? someValue(g_random) : float(random(5)) / 4.0f;
        const float most = random(4) == 0 ? someValue(g_random) : least + float(1 + random(4)) / 2.0f;
        setFloat(0x55d05c + 4 * i, least);
        setFloat(0x55d040 + 4 * i, most);
        setFloat(0x55d024 + 4 * i, least + (most - least) * someValue(g_random));
        setFloat(0x5e10e0 + 4 * i, random(3) == 0 ? someValue(g_random) : least + (most - least) * someValue(g_random));
        setFloat(0x5e10fc + 4 * i, someValue(g_random));
    }
    g_fixedSlider = std::numeric_limits<float>::quiet_NaN();
    if (random(5) == 0)
    {
        g_fixedSlider = 0.5f;
        for (x86::reg32 i = 0; i < 7; ++i)
            setFloat(0x5e10fc + 4 * i, random(8) == 0 ? 0.25f : 0.5f);
    }
    // The advanced tunings, sometimes the same.
    if (random(2))
        std::memcpy(g_memory + 0x8b9200, g_memory + 0x8b9b60, 50 * 0x30);
    if (random(2))
        std::memcpy(g_memory + 0x8b88a0, g_memory + 0x8b9b60, 50 * 0x30);
    if (random(3) == 0)
        g_memory[0x8b9b60 + random(50 * 0x30)] ^= x86::reg8(1 + random(255));

    // The tracks' screens: the track, its places and its art's names.
    setWord32(0x6fd4ac, random(13));
    std::memcpy(g_memory + 0x5f3f00, "place\0road%d\0", 15);
    std::memcpy(g_memory + 0x5f3f20, "trk00", 6);
    for (x86::reg32 t = 0; t < 13; ++t)
    {
        setWord32(0x5597f4 + 4 * t, random(5) == 0 ? x86::reg32(-1) : random(5));
        setWord32(0x559620 + 4 * t, 0x5f3f20);
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord32(0x5596d4 + 16 * t + 4 * i, random(4) == 0 ? 0x5f3f06 : 0x5f3f00);
    }
    std::memcpy(g_memory + 0x7a31b4, "fedata\\art\\", 12);
    for (x86::reg32 l = 0; l < 5; ++l)
        std::strcpy(reinterpret_cast<char *>(g_memory) + 0x7a2b34 + 0x80 * l, random(2) ? "speech\\" : "speech2\\");
    for (x86::reg32 l = 0; l < 8; ++l)
        setWord32(0x560b34 + 4 * l, 0x5f3f20);
    setWord32(0x559828, random(2));
    setWord32(0x7dd838, random(2));
    setWord32(0x7dd84c, random(2) ? 0 : random(0));
    setWord32(0x7a22e0, random(3) == 0);

    // The main menu's: the aids, the network game, mainmult's race list and
    // its name after it, which the screen checks.
    for (x86::reg32 i = 0; i < 6; ++i)
        setWord32(0x6fd474 + 4 * i, random(4) == 0 ? random(0) : 0);
    setWord32(0x6fd48c, random(2) ? 0 : random(0));
    setWord32(0x7a22d8, random(0));
    g_memory[0x7a22da] = x86::reg8(random(2));
    g_memory[0x7a22db] = x86::reg8(random(4) == 0);
    g_memory[0x7a22dc] = x86::reg8(random(3) == 0);
    g_memory[0x7a22dd] = x86::reg8(random(4) == 0);
    setWord32(0x7a22e4, random(4));
    setWord32(0x6fd50c, random(2));
    setWord32(0x5556d8, random(5) == 0 ? random(0) : random(7));
    setWord32(0x5556dc, random(2) ? word32(0x5556d8) : random(7));
    setWord32(0x6fd470, random(3));
    for (x86::reg32 i = 0; i < 7; ++i)
        g_memory[0x555730 + 2 * i] = x86::reg8(random(2)), g_memory[0x555731 + 2 * i] = x86::reg8(random(8) == 0);
    std::memcpy(g_memory + 0x55573e, "opcars", 7);
    setWord32(0x664658, random(2));
#include "group_world.inc"
}

x86::CPU entryState()
{
    x86::CPU cpu;
    std::memset(&cpu, 0, sizeof cpu);
    cpu.init(0, 0);
    cpu.eax = random(0);
    cpu.ebx = random(0);
    cpu.ecx = random(0);
    cpu.edx = random(0);
    cpu.esi = random(0);
    cpu.edi = random(0);
    cpu.ebp = random(0);
    cpu.esp = kStack;
    cpu.fpu.setControl(0x007f);
    // Something on the x87 already, as the menus leave it.
    for (unsigned i = random(3); i > 0; --i)
        cpu.fpu.push(x86::Float(random(1000)));
    cpu.flags.lo = x86::reg8((random(0) & 0xd5) | 0x02);
    return cpu;
}

struct Totals
{
    unsigned cases = 0;
    unsigned failures = 0;
};

std::map<std::string, Totals> g_totals;
std::vector<x86::reg8> g_before;
std::vector<x86::reg8> g_original;

constexpr x86::reg32 kPage = 0x1000;

/* The pages written since the last time this was asked, added to `pages`.  Copying
 * and comparing the whole 16 MB four times a case, in every process at once, was
 * nearly all the time the checks took: the machine's memory was what they waited on. */
void takeWritten(std::vector<x86::reg32>& pages)
{
#ifdef _WIN32
    static std::vector<void*> found(kMemory / kPage);
    ULONG_PTR count = found.size();
    DWORD granularity = 0;
    if (GetWriteWatch(WRITE_WATCH_FLAG_RESET, g_memory, kMemory, found.data(), &count, &granularity) != 0)
    {
        std::printf("GetWriteWatch failed: %lu\n", GetLastError());
        std::fflush(stdout);
        std::abort();
    }
    for (ULONG_PTR i = 0; i < count; ++i)
        pages.push_back(x86::reg32(static_cast<x86::reg8*>(found[i]) - g_memory));
#else
    for (x86::reg32 page = 0; page < kMemory; page += kPage)
        pages.push_back(page);
#endif
}

/* One case: the generated function at `address`, then `native`, from the same
 * memory, registers and script. */
void compare(nfs3hp::Application& app, const char* name, x86::reg32 address, Native native, const x86::CPU& entry)
{
    Totals& totals = g_totals[name];
    ++totals.cases;
    // g_before and g_original as the memory is now: whatever changed since they last
    // were, which is what the last case wrote and what this one's world did.
    static std::vector<x86::reg32> pending, scratch;
    if (g_before.empty())
    {
        g_before.assign(g_memory, g_memory + kMemory);
        g_original.assign(g_memory, g_memory + kMemory);
        pending.clear();
        takeWritten(pending);
    }
    else
    {
        takeWritten(pending);
        for (x86::reg32 page : pending)
        {
            std::memcpy(&g_before[page], g_memory + page, kPage);
            std::memcpy(&g_original[page], g_memory + page, kPage);
        }
    }
    pending.clear();
    const unsigned seed = random(0);

    g_testing = address;
    g_caseAddress = address;
    x86::CPU original = entry;
    g_script.seed(seed);
    g_log.clear();
    g_calls = 0;
    app.dynamic_call(address, original);
    const std::vector<x86::reg32> originalLog = g_log;
    takeWritten(pending);
    for (x86::reg32 page : pending)
    {
        std::memcpy(&g_original[page], g_memory + page, kPage);
        std::memcpy(g_memory + page, &g_before[page], kPage);
    }
    scratch.clear();
    takeWritten(scratch);  // what was just put back: in `pending` already

    x86::CPU mine = entry;
    g_script.seed(seed);
    g_log.clear();
    g_calls = 0;
    const bool took = native(&app, mine);

    char what[400] = "";
    // The port's own change: mainsplt's rows stand higher while it is up
    // (native_frontend_main.cpp), back where the game has them as it closes.
    // Checked here, then the generated code's rows taken for the rest.
    const bool splitEnter = std::strstr(name, "splitMenuOnEnter") != nullptr;
    if (took && (splitEnter || std::strstr(name, "splitMenuOnExit")))
    {
        static const std::int16_t kSplitX[] = { 18, 8, 2, 0, 1, 3, 10, 19 };
        static const std::int16_t kSplitY[] = { 115, 150, 185, 220, 255, 290, 325, 360 };
        static const std::int16_t kGameX[] = { 12, 5, 2, 1, 4, 9, 16, 27 };
        static const std::int16_t kGameY[] = { 138, 173, 208, 243, 278, 313, 348, 383 };
        const bool rowsMoved = splitEnter;
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            std::int16_t x, y;
            std::memcpy(&x, g_memory + kRowX + 4 * i, 2);
            std::memcpy(&y, g_memory + kRowY + 4 * i, 2);
            if (x != (rowsMoved ? kSplitX : kGameX)[i] || y != (rowsMoved ? kSplitY : kGameY)[i])
                std::snprintf(what, sizeof what, "row %u at %d,%d", unsigned(i), x, y);
        }
        for (x86::reg32 at : { kRowX, kRowY })
            std::memcpy(g_memory + at, &g_original[at], 4 * 8);
    }
    // Written by either: the rest of the memory is as it was on both sides.
    takeWritten(pending);
    std::sort(pending.begin(), pending.end());
    pending.erase(std::unique(pending.begin(), pending.end()), pending.end());
    if (what[0])
    {
    }
    else if (!took)
    {
        std::snprintf(what, sizeof what, "the native stepped aside");
    }
    else if (g_log != originalLog)
    {
        std::size_t i = 0;
        while (i < g_log.size() && i < originalLog.size() && g_log[i] == originalLog[i])
            ++i;
        std::size_t call = 0;
        x86::reg32 callee = 0;
        for (std::size_t j = 0; j < i && j < originalLog.size(); ++j)
            if (originalLog[j] == 0xca110000 && j + 1 < originalLog.size())
                ++call, callee = originalLog[j + 1];
        std::snprintf(what, sizeof what,
                      "calls differ at word %zu (after %zu calls, last sub_%06x): native %08x, generated %08x "
                      "(lengths %zu/%zu)",
                      i, call, unsigned(callee), i < g_log.size() ? unsigned(g_log[i]) : 0u,
                      i < originalLog.size() ? unsigned(originalLog[i]) : 0u, g_log.size(), originalLog.size());
    }
    else
    {
        const x86::reg32 stackEnd = original.esp;
        const x86::reg32 stackStart = original.esp - 0x10000;
        unsigned differing = 0;
        for (x86::reg32 chunk : pending)
        {
            if (std::memcmp(g_memory + chunk, &g_original[chunk], kPage) == 0)
                continue;
            for (x86::reg32 i = chunk; i < chunk + kPage; ++i)
            {
                if (g_memory[i] == g_original[i] || (i >= stackStart && i < stackEnd))
                    continue;
                if (differing++ == 0)
                    std::snprintf(what, sizeof what, "memory at %08x: %02x native, %02x generated", unsigned(i),
                                  unsigned(g_memory[i]), unsigned(g_original[i]));
            }
        }
        if (differing > 1)
        {
            const std::size_t length = std::strlen(what);
            std::snprintf(what + length, sizeof what - length, " (%u bytes)", differing);
        }
        if (!what[0]
            && (mine.eax != original.eax || mine.ebx != original.ebx || mine.ecx != original.ecx
                || mine.edx != original.edx || mine.esi != original.esi || mine.edi != original.edi
                || mine.ebp != original.ebp || mine.esp != original.esp))
            std::snprintf(what, sizeof what,
                          "registers native/generated: eax %08x/%08x ebx %08x/%08x ecx %08x/%08x edx %08x/%08x "
                          "esi %08x/%08x edi %08x/%08x ebp %08x/%08x esp %08x/%08x",
                          unsigned(mine.eax), unsigned(original.eax), unsigned(mine.ebx), unsigned(original.ebx),
                          unsigned(mine.ecx), unsigned(original.ecx), unsigned(mine.edx), unsigned(original.edx),
                          unsigned(mine.esi), unsigned(original.esi), unsigned(mine.edi), unsigned(original.edi),
                          unsigned(mine.ebp), unsigned(original.ebp), unsigned(mine.esp), unsigned(original.esp));
        if (!what[0] && mine.fpu.count != original.fpu.count)
            std::snprintf(what, sizeof what, "x87 depth native %u, generated %u",
                          unsigned(mine.fpu.count), unsigned(original.fpu.count));
        if (!what[0])
        {
            for (unsigned i = 0; i < mine.fpu.count && i < 8; ++i)
            {
                if (std::memcmp(&mine.fpu.st(i), &original.fpu.st(i), sizeof(x86::Float)) != 0)
                {
                    std::snprintf(what, sizeof what, "x87 st(%u) differs", i);
                    break;
                }
            }
        }
    }
    if (what[0])
    {
        if (std::getenv("OPTIONS_CHECKS_DUMP") && totals.failures == 0)
        {
            std::printf("generated:");
            for (x86::reg32 v : originalLog)
                std::printf(v == 0xca110000 ? "\n " : " %08x", unsigned(v));
            std::printf("\nnative:");
            for (x86::reg32 v : g_log)
                std::printf(v == 0xca110000 ? "\n " : " %08x", unsigned(v));
            std::printf("\n");
        }
        ++totals.failures;
        if (totals.failures <= 5)
            std::printf("FAIL %s case %u: %s\n", name, totals.cases, what);
    }
}

/* How a function is entered. */
enum class Entry
{
    Menu,          // eax a screen, sometimes 0
    Key,           // eax an item, edx the screen, bx a key
    Item,          // eax an item, edx 0 or 1
    None,
    Answer,        // eax an answer, edx where the byte goes
    ApplyAxis,     // the stack
    ShowController,
    Advanced,      // eax and edx advanced tunings, or 0
    MenuAndMode,   // eax a screen, edx whom the tuning is for
    Numbers,       // eax and edx small numbers
    ShowItems,     // eax the screen, its items' show handlers some of them
    CurrentName,   // the screen shown, or none
    LinesBox,      // eax lines, edx them, ebx and ecx a row and a field below, the rect on the stack
    ButtonRowBox,  // eax buttons, edx them, ebx the rect
    PlaceButtons,  // eax buttons, edx them, ebx a column, ecx the rect
    PointAt,       // eax a button, the count of them
    Question,      // a question's lines and buttons, its help, focus, column and callback
    Message,       // a message's lines, its time and keys, its callback
    MessageById,   // eax a line of the language's
    Edit,          // a line to type in: its lines, buttons, focus, source, most, mode and callback
    FindItem,      // eax the screen, edx a name to look its items' codelinks up by
};

/* A dialog's texts: an array of `count` at `array`, each a text or now and
 * then none, made up from `strings` on. */
constexpr x86::reg32 kDialogLines = 0x677000, kDialogButtons = 0x677040, kDialogHelp = 0x677080;
constexpr x86::reg32 kDialogStrings = 0x677100, kDialogSource = 0x677900, kDialogRect = 0x677a00;

void makeTexts(x86::reg32 array, x86::reg32 count, x86::reg32 strings, bool gaps)
{
    for (x86::reg32 i = 0; i < count && i < 12; ++i)
    {
        const x86::reg32 text = strings + 0x40 * i;
        const x86::reg32 length = random(20);
        for (x86::reg32 c = 0; c < length; ++c)
            g_memory[text + c] = x86::reg8('A' + random(26));
        g_memory[text + length] = 0;
        setWord32(array + 4 * i, gaps && random(6) == 0 ? 0 : text);
    }
}

/* Some lines and buttons for a dialog: their counts in eax and ebx, their
 * arrays in edx and ecx -- none, now and then; ten buttons or more. */
void dialogTexts(x86::CPU& cpu)
{
    const x86::reg32 lines = random(8) == 0 ? 0 : 1 + random(5);
    const x86::reg32 buttons = random(8) == 0 ? 0 : random(10) == 0 ? 10 + random(3) : 1 + random(4);
    makeTexts(kDialogLines, lines, kDialogStrings, false);
    makeTexts(kDialogButtons, buttons, kDialogStrings + 0x40 * 6, true);
    makeTexts(kDialogHelp, buttons, kDialogStrings + 0x40 * 18, true);
    cpu.eax = lines;
    cpu.edx = random(12) == 0 ? 0 : kDialogLines;
    cpu.ebx = buttons;
    cpu.ecx = random(12) == 0 ? 0 : kDialogButtons;
}

/* A rect of floats, top, left, bottom, right, on the 640 by 480 screen. */
void makeRect(x86::reg32 rect)
{
    const x86::reg32 left = random(320), top = random(240);
    setFloat(rect + 4, float(left));
    setFloat(rect + 0, float(top));
    setFloat(rect + 0xc, float(left + 60 + random(320)));
    setFloat(rect + 8, float(top + 30 + random(240)));
}

struct Case
{
    const char* name;
    x86::reg32 address;
    Native native;
    Entry entry;
};

/* The screens' own sliders, by screen: an item handed to one of their
 * functions is one of them most of the time. */
const x86::reg32 kAudioSlots[] = { 0x554f3c, 0x554f40, 0x554f44, 0x554f48, 0x554f4c };
const x86::reg32 kAxisSlots[] = { 0x558a10, 0x558a14, 0x558a18, 0x558a1c, 0x558a20, 0x558a24, 0x558a28, 0x558a2c };
const x86::reg32 kForceSlots[] = { 0x6611e0, 0x6611e4, 0x6611e8, 0x6611ec, 0x6611f0, 0x6611f4 };

x86::reg32 someItem(x86::reg32 address)
{
    const x86::reg32* slots = nullptr;
    std::size_t count = 0;
    if (address >= 0x437990 && address < 0x438300)
        slots = kAudioSlots, count = std::size(kAudioSlots);
    else if (address >= 0x442d00 && address < 0x443100)
        slots = kAxisSlots, count = std::size(kAxisSlots);
    else if (address >= 0x447b00 && address < 0x447e00)
        slots = kForceSlots, count = std::size(kForceSlots);
    if (slots && random(4) != 0)
    {
        const x86::reg32 slot = slots[random(count)];
        if (word32(slot) == 0)
            setWord32(slot, poolItem(random(kPoolItems)));
        return word32(slot);
    }
    return poolItem(random(kPoolItems));
}

x86::CPU caseEntry(Entry entry, x86::reg32 address)
{
    x86::CPU cpu = entryState();
    switch (entry)
    {
    case Entry::Menu:
        cpu.eax = random(10) == 0 ? 0 : g_menu;
        break;
    case Entry::Key:
        cpu.eax = random(12) == 0 ? 0 : someItem(address);
        cpu.edx = random(12) == 0 ? 0 : g_menu;
        cpu.ebx = (random(0) & 0xffff0000) | (random(2) ? 0x0d : random(4) ? 0x1b : random(0x10000));
        break;
    case Entry::Item:
        cpu.eax = someItem(address);
        cpu.edx = random(3) == 0 ? random(0) : random(2);
        break;
    case Entry::None:
        break;
    case Entry::FindItem:
    {
        // The screen's items end where an item's type is 0: somewhere in the
        // pool, or past it.
        cpu.eax = g_menu;
        const std::uint32_t end = random(3) == 0 ? kPoolItems + random(10) : random(kPoolItems + 10);
        setWord32(kItems + kItemSize * (kPoolFirst + end), 0);
        makeTexts(kDialogLines, 1, kDialogStrings, false);
        cpu.edx = random(8) == 0 ? 0 : word32(kDialogLines);
        break;
    }
    case Entry::Answer:
        cpu.eax = random(5);
        cpu.edx = random(6) == 0 ? 0 : 0x5f3e00 + random(16);
        break;
    case Entry::ApplyAxis:
        setFloat(cpu.esp + 4, someValue(g_random));
        setFloat(cpu.esp + 8, someValue(g_random));
        setWord32(cpu.esp + 12, random(4));
        setWord32(cpu.esp + 16, random(8));
        break;
    case Entry::ShowController:
        cpu.eax = random(10) == 0 ? 0 : g_menu;
        cpu.edx = random(4);
        break;
    case Entry::Advanced:
    {
        static const x86::reg32 kTunings[] = { 0x8b9b60, 0x8b9200, 0x8b88a0, 0 };
        cpu.eax = kTunings[random(4)];
        cpu.edx = kTunings[random(4)];
        break;
    }
    case Entry::MenuAndMode:
        cpu.eax = random(10) == 0 ? 0 : g_menu;
        cpu.edx = random(6);
        break;
    case Entry::Numbers:
        cpu.eax = random(8);
        cpu.edx = random(14);
        break;
    case Entry::ShowItems:
    {
        // The pool's items with their show handlers or none, after a race
        // only or before one only now and then; past them the table's end.
        for (std::uint32_t i = 0; i < kPoolItems + 10; ++i)
        {
            const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
            setWord32(item + 0x24, random(3) == 0 ? kShowHandler : 0);
            g_memory[item + 0x14] = x86::reg8(random(4) == 0), g_memory[item + 0x15] = 0;
            g_memory[item + 0x16] = x86::reg8(random(4) == 0), g_memory[item + 0x17] = 0;
        }
        setWord32(kItems + kItemSize * (kPoolFirst + kPoolItems + 10), 0);
        setWord32(0x662968, random(2));
        cpu.eax = g_menu;
        break;
    }
    case Entry::CurrentName:
        setWord32(0x559230, random(4) == 0 ? 0 : g_menu);
        setWord32(g_menu + 0x14, 0x5f3f20);
        break;
    case Entry::LinesBox:
        dialogTexts(cpu);
        cpu.ebx = random(2) ? 0 : random(2) ? 1 : random(0);
        cpu.ecx = random(2) ? 0 : random(2) ? 1 : random(0);
        setWord32(cpu.esp + 4, kDialogRect);
        break;
    case Entry::ButtonRowBox:
        dialogTexts(cpu);
        cpu.eax = cpu.ebx, cpu.edx = cpu.ecx;
        cpu.ebx = kDialogRect;
        break;
    case Entry::PlaceButtons:
        dialogTexts(cpu);
        cpu.eax = cpu.ebx, cpu.edx = cpu.ecx;
        cpu.ebx = random(2) ? 0 : random(2) ? 1 : random(0);
        cpu.ecx = kDialogRect;
        makeRect(kDialogRect);
        setWord32(0x558b40, random(640));
        break;
    case Entry::PointAt:
        setWord32(0x558b30, random(12));
        cpu.eax = x86::reg32(random(14)) - 2;
        break;
    case Entry::Question:
        dialogTexts(cpu);
        setWord32(cpu.esp + 4, random(3) == 0 ? 0 : kDialogHelp);
        setWord32(cpu.esp + 8, x86::reg32(random(8)) - 2);
        setWord32(cpu.esp + 12, random(2) ? 0 : random(2) ? 1 : random(0));
        setWord32(cpu.esp + 16, random(0));
        break;
    case Entry::Message:
        dialogTexts(cpu);
        cpu.ebx = random(5) == 0 ? 0 : random(2) ? x86::reg32(0) - random(1000) : random(1000);
        cpu.ecx = random(0);
        setWord32(cpu.esp + 4, random(0));
        break;
    case Entry::MessageById:
        cpu.eax = random(0x900);
        break;
    case Entry::Edit:
    {
        dialogTexts(cpu);
        setWord32(0x558c5c, random(5) == 0 ? random(0) : random(4));
        const x86::reg32 length = random(20);
        for (x86::reg32 c = 0; c < length; ++c)
            g_memory[kDialogSource + c] = x86::reg8('a' + random(26));
        g_memory[kDialogSource + length] = 0;
        setWord32(cpu.esp + 4, x86::reg32(random(8)) - 2);
        setWord32(cpu.esp + 8, random(6) == 0 ? 0 : kDialogSource);
        setWord32(cpu.esp + 12, random(5) == 0 ? 0 : random(5) == 0 ? 0x100 + random(0x200) : random(16));
        setWord32(cpu.esp + 16, random(5));
        setWord32(cpu.esp + 20, random(0));
        break;
    }
    }
    return cpu;
}

}

#define CASE(name, address, entry) { #name " sub_" #address, 0x##address, nfs3hp::name, Entry::entry }
#define WHOLE(name, address, entry) \
    { #name " sub_" #address, 0x##address, whole<nfs3hp::frontend::name>, Entry::entry }

int main(int argc, char** argv)
{
    nfs3hp::Application app("frontend_checks");
    loadImage(argc > 1 ? argv[1] : "nfs3hp/nfs3.exe");
    g_image.assign(g_memory + 0x534000, g_memory + 0x56e000);
    const unsigned rounds = argc > 2 ? unsigned(std::atoi(argv[2])) : 1000;
    // A share of the functions, "k/n" -- every n-th from the k-th -- for
    // windows/run.py to check them side by side.
    unsigned share = 0, shares = 1;
    if (argc > 3)
        std::sscanf(argv[3], "%u/%u", &share, &shares);

    const Case cases[] = {
#include "group_cases.inc"
        WHOLE(optionsOnEnter, 442aa0, Menu),
        WHOLE(feedOnEnter, 4374b0, Menu),
        WHOLE(feedOnExit, 437590, Menu),
        WHOLE(feedSeparate, 437250, None),
        WHOLE(feedTimeButtonKey, 437300, Key),
        WHOLE(feedPositionButtonKey, 437390, Key),
        WHOLE(feedSpeedButtonKey, 437420, Key),
        WHOLE(feedTimeListKey, 437350, Key),
        WHOLE(feedPositionListKey, 4373e0, Key),
        WHOLE(feedSpeedListKey, 437470, Key),
        WHOLE(funkeysOnEnter, 437600, Menu),
        WHOLE(funkeysOnExit, 4377c0, Menu),
        WHOLE(hardkeysOnEnter, 4377d0, Menu),
        WHOLE(audioOnEnter, 437e00, Menu),
        WHOLE(audioOnExit, 438070, Menu),
        WHOLE(audioOnFrame, 438260, Menu),
        WHOLE(audioVolumeDraw, 437a90, Item),
        WHOLE(audioMenuMusicMoved, 437990, Item),
        WHOLE(audioRaceMusicMoved, 4379b0, Item),
        WHOLE(audioEffectsMoved, 4379d0, Item),
        WHOLE(audioEngineMoved, 437a20, Item),
        WHOLE(audioSpeechMoved, 437a70, Item),
        WHOLE(controlsOnEnter, 43d4d0, Menu),
        WHOLE(controlsOnExit, 43d5f0, Menu),
        WHOLE(controlsCancelKey, 43d3f0, Key),
        WHOLE(controlsResetClicked, 43d460, Item),
        WHOLE(controlsResetAnswered, 43d410, Answer),
        WHOLE(deadZoneFillList, 43d9a0, None),
        WHOLE(deadZoneOnEnter, 4432c0, Menu),
        WHOLE(deadZoneOnExit, 443430, Menu),
        WHOLE(deadZoneCancelKey, 442db0, Key),
        WHOLE(deadZoneChooseKey, 443260, Key),
        WHOLE(deadZoneChooseKeyInRace, 443290, Key),
        WHOLE(deadZoneShowController, 443070, ShowController),
        WHOLE(deadZoneAxisIndex, 442e60, Item),
        WHOLE(deadZoneApplyAxis, 442dd0, ApplyAxis),
        WHOLE(deadZoneAxisMoved, 442ef0, Item),
        WHOLE(deadZoneAxisDraw, 442ff0, Item),
        WHOLE(forceOnEnter, 447dd0, Menu),
        WHOLE(forceOnExit, 4481c0, Menu),
        WHOLE(forceCancelKey, 447b40, Key),
        WHOLE(forceStrengthMoved, 447b60, Item),
        WHOLE(forceRoadEffectsMoved, 447c40, Item),
        WHOLE(forceCollisionMoved, 447c70, Item),
        WHOLE(forceEngineMoved, 447ca0, Item),
        WHOLE(forceBodyRollMoved, 447cd0, Item),
        WHOLE(forceRoadGripMoved, 447d50, Item),
        WHOLE(graphicsOnEnter, 448440, Menu),
        WHOLE(graphicsOnExit, 4484e0, Menu),
        WHOLE(graphicsOnFrame, 448430, Menu),
        WHOLE(advancedOnEnter, 448610, Menu),
        WHOLE(advancedOnExit, 448770, Menu),
        WHOLE(advancedOnFrame, 4485a0, Menu),
        WHOLE(advancedIntensityMoved, 448570, Item),
        // The cars' screens.
        WHOLE(carMenuMusicBack, 4104b0, None),
        WHOLE(carSaveFrame, 4267a0, Numbers),
        WHOLE(trackSaveFrame, 4267f0, Numbers),
        WHOLE(carRecord, 426870, None),
        WHOLE(trackRecord, 426b00, None),
        WHOLE(car1OnEnter, 426d60, Menu),
        WHOLE(car2OnEnter, 426ef0, Menu),
        WHOLE(car1OnFrame, 426e20, Menu),
        WHOLE(car2OnFrame, 426f80, Menu),
        WHOLE(car1OnExit, 426ee0, Menu),
        WHOLE(car2OnExit, 427040, Menu),
        WHOLE(car1EditClicked, 426c20, None),
        WHOLE(car2EditClicked, 426c40, None),
        WHOLE(car1ColourClicked, 426c60, None),
        WHOLE(car2ColourClicked, 426ce0, None),
        WHOLE(opponentFitCar, 427050, Menu),
        WHOLE(opponentFitArt, 4270c0, Menu),
        WHOLE(opponentOnEnter, 4272a0, Menu),
        WHOLE(opponentOnExit, 4272e0, Menu),
        WHOLE(opponentOnFrame, 427190, Menu),
        WHOLE(tuningLoad, 4272f0, Numbers),
        WHOLE(tuningSave, 427370, Numbers),
        WHOLE(tuningEngineMoved, 4273e0, Item),
        WHOLE(tuningBrakesMoved, 427470, Item),
        WHOLE(tuningSteeringMoved, 427500, Item),
        WHOLE(tuningGearsMoved, 427590, Item),
        WHOLE(tuningSuspensionMoved, 427620, Item),
        WHOLE(tuningAeroMoved, 4276b0, Item),
        WHOLE(tuningTiresMoved, 427740, Item),
        WHOLE(tuningSetSliders, 4277d0, Menu),
        WHOLE(tuningRememberSliders, 4279b0, Menu),
        WHOLE(tuningSlidersMoved, 427ac0, Menu),
        WHOLE(tuningBindSliders, 427c20, Menu),
        WHOLE(tuningLoadAdvanced, 427d00, None),
        WHOLE(tuningDefaultsAnswered, 427d80, Answer),
        WHOLE(tuningDoneKey, 427e10, Key),
        WHOLE(tuningCancelKey, 427ea0, Key),
        WHOLE(tuningCarListKey, 427f40, Key),
        WHOLE(tuningDefaultsKey, 427fb0, Key),
        WHOLE(tuningBindDoneCancel, 428020, Menu),
        WHOLE(tuningFitButtons, 428060, MenuAndMode),
        WHOLE(tuningPrepare, 4281e0, Menu),
        WHOLE(tuningOnEnter, 428360, Menu),
        WHOLE(tuningOnExit, 4283b0, Menu),
        WHOLE(tuningAdvancedDiffers, 4dcaf0, Advanced),
        WHOLE(tuningCopyAdvanced, 4dcb60, Advanced),
        // The tracks'.
        WHOLE(locationOnEnter, 4487e0, Menu),
        WHOLE(locationOnFrame, 4487d0, Menu),
        WHOLE(locationOnExit, 4489e0, Menu),
        WHOLE(infoPlaceTrackPlaces, 456200, None),
        WHOLE(infoOnEnter, 4568d0, Menu),
        WHOLE(infoOnFrame, 456890, Menu),
        WHOLE(infoOnExit, 456990, Menu),
        WHOLE(infoSpeak, 4c94e0, Numbers),
        WHOLE(infoStopSpeaking, 4c9590, None),
        // The main menu: main, mainsplt, mainmult.
        WHOLE(mainMenuOnEnter, 45d050, Menu),
        WHOLE(mainMenuOnExit, 45d2f0, Menu),
        WHOLE(mainMenuOnFrame, 45cff0, Menu),
        WHOLE(mainMenuOnPlayerName, 45cef0, None),
        WHOLE(mainMenuPlayerNameAnswer, 45cdd0, Answer),
        WHOLE(splitMenuOnEnter, 45d360, Menu),
        WHOLE(splitMenuOnExit, 45d620, Menu),
        WHOLE(splitMenuOnFrame, 45d300, Menu),
        WHOLE(splitMenuOnPlayer2Name, 45cf70, None),
        WHOLE(splitMenuPlayer2NameAnswer, 45ce60, Answer),
        WHOLE(multiMenuOnEnter, 440470, Menu),
        WHOLE(multiMenuOnExit, 4405b0, Menu),
        WHOLE(multiMenuOnFrame, 4403f0, Menu),
        WHOLE(multiMenuApplyRaceList, 440160, Menu),
        WHOLE(multiMenuFollowRaceType, 440390, None),
        WHOLE(multiMenuOnBack, 4400d0, None),
        WHOLE(multiMenuLeaveAnswer, 43fcb0, Answer),
        // What the main menu calls of the menu loop and the dialogs, native whole.
        WHOLE(menuShowItems, 44a960, ShowItems),
        WHOLE(menuShownNow, 44a660, Item),
        WHOLE(menuFindItem, 442a40, FindItem),
        WHOLE(dialogFocusedButton, 4446c0, None),
        WHOLE(menuCurrentName, 44ae00, CurrentName),
        WHOLE(dialogLinesBox, 443a90, LinesBox),
        WHOLE(dialogButtonRowBox, 443c00, ButtonRowBox),
        WHOLE(dialogPlaceButtons, 443d20, PlaceButtons),
        WHOLE(dialogPointAtButton, 444570, PointAt),
        WHOLE(dialogOpenQuestion, 4446d0, Question),
        WHOLE(dialogOpenMessage, 444a80, Message),
        WHOLE(dialogMessageById, 444be0, MessageById),
        WHOLE(dialogEditText, 444c10, None),
        WHOLE(dialogOpenEdit, 444c20, Edit),
        WHOLE(dialogFadedIn, 445f00, None),
    };

    for (unsigned round = 0; round < rounds; ++round)
    {
        for (std::size_t i = 0; i < std::size(cases); ++i)
        {
            char address[16];
            std::snprintf(address, sizeof address, "%x", unsigned(cases[i].address));
            if (const char* only = std::getenv("FRONTEND_CHECKS_ONLY"))
                if (!std::strstr(only, address))
                    continue;
            if (const char* skip = std::getenv("FRONTEND_CHECKS_SKIP"))
                if (std::strstr(skip, address))
                    continue;
            if (shares > 1 && i % shares != share)
                continue;
            const Case& c = cases[i];
            makeWorld();
            x86::CPU entry = caseEntry(c.entry, c.address);
#include "group_entry.inc"
            compare(app, c.name, c.address, c.native, entry);
        }
    }
    unsigned failures = 0;
    for (const auto& entry : g_totals)
    {
        if (entry.second.failures)
            std::printf("%-40s %5u cases, %u differ\n", entry.first.c_str(), entry.second.cases,
                        entry.second.failures);
        failures += entry.second.failures;
    }
    const std::size_t checked = shares > 1 ? (std::size(cases) + shares - 1 - share) / shares : std::size(cases);
    std::printf("%zu functions, %u cases each: %s\n", checked, rounds,
                failures ? "FAILED" : "all match");
    return failures ? 1 : 0;
}
