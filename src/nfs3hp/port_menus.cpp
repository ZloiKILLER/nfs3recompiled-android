#include "port_menus.h"
#include "native_ea_formats.h"
#include "native_watcom.h"
#include "port_buttons.h"
#include "port_text.h"

#include <cstdint>
#include <cstring>

/* The menu files and the screens they make, rewritten (port.h says how).
 *
 * A .MNU is a list of items, each a section and its keys:
 *
 *   ; a comment
 *   [button]
 *   x = 120
 *   y = 200
 *   codelink = MAIN_LOCATION_BUTTON
 *   text = kTextTrack
 *   nextmenu = location
 *
 * The section names the widget (the registry at 0x557a84: type, name, the
 * item a new one starts as), whose template the new item starts from.  A key
 * is looked up in the parser's table at 0x557c10 -- the widget it belongs to
 * (1 for every widget), its name, its kind, the item's field it fills -- and
 * its value stored there by kind: a number of 1, 2 or 4 bytes, a string kept
 * in the menus' string pool, a line of the language's text, a float.  Spaces
 * and tabs in a key are dropped; a value starts at its first other
 * character.  Names are matched as written, case and all -- widgets, keys,
 * screens, codelinks, text names (strcmp) -- but for a screen's handlers,
 * found by its name in any case.  A button's nextmenu loads that screen too,
 * and the ones it leads to, so opening the first screen reads every one
 * reachable from it.  The screens are 0x24 bytes each from 0x6040d0 (their
 * name, their handlers, their first item), the items 0xbc bytes each from
 * 0x604ee0.
 *
 * Quirks of the original, kept: a number is taken only when every character
 * of it is a digit, so a negative one reads as 0; a line that is neither a
 * section nor a key = value -- a key with nothing after its '=' included --
 * calls the game's key wait (0x4f0120), and the line is skipped.
 *
 * Each function is a typed core and a thunk under the game's name.  The
 * cores call each other as C++, each given the esp its game's code is entered
 * at, so that what the game's calls out of it find on the stack -- the C
 * library's strtok keeps a pointer into the stack between calls -- is where
 * it was; what is not rewritten (the C library, the game's key wait) is
 * called by address, the language's text and the menus' lists as C++ through
 * their thunks.  The game's code leaves ebx, ecx and edx otherwise than it
 * got them in several of these (the scratch of a Watcom function); no caller
 * reads them after (by liveness on nfs3.exe), so each thunk gives back
 * every register but eax as the caller had it. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{
namespace port
{
namespace menus
{

namespace
{

using x86::Float;

/* ---- The records ---- */

#pragma pack(push, 1)
/* A row of the widgets' registry: its type, its name, the item a new one of
 * it starts as (0 past the last). */
struct Widget
{
    reg32 type, name, templ;
};

/* A row of the parser's table: the widget it belongs to (1 every one), the
 * key's name, the kind of its value, the field it fills (an address in the
 * first item). */
struct Keyword
{
    reg32 widget, name, kind, field;
};

/* A row of the screens' handlers by name (0 past the last): the name, then
 * the five handlers a screen has (its first five words). */
struct Handlers
{
    reg32 name, handlers[5];
};

/* A screen: its handlers (onLoad, then three the loop calls, the last of
 * them every pass), its name, its first item, the item selected, and a word
 * of whether its onLoad ran at 0x20. */
struct Menu
{
    reg32 handlers[5];
    reg32 name;       // 0x14
    int16_t firstItem;  // 0x18
    int16_t selected;   // 0x1a
    uint8_t unused1c[2];
    int16_t edited;     // 0x1e
    int16_t loaded;     // 0x20
    uint8_t unused22[2];
};
#pragma pack(pop)
static_assert(sizeof(Widget) == 12, "a widget's row is 12 bytes");
static_assert(sizeof(Keyword) == 16, "a keyword's row is 16 bytes");
static_assert(sizeof(Handlers) == 24, "a handlers' row is 24 bytes");
static_assert(sizeof(Menu) == 0x24, "a screen is 0x24 bytes");

/* How a key's value is stored (the third word of its row). */
enum Kind : reg32
{
    kByte = 0,
    kWord = 1,
    kDword = 2,
    kLong = 3,
    kString = 4,
    kText = 5,
    kFloat = 6,
};

/* ---- The game's variables ---- */

constexpr Records<Widget> kWidgets{0x557a84};
constexpr Records<Keyword> kKeywords{0x557c10};
constexpr Records<Handlers> kHandlerTable{0x555c48};
constexpr Records<Menu> kMenus{0x6040d0};
// The items: 0xbc bytes each; the first word is the widget's type.
constexpr reg32 kItems = 0x604ee0, kItemBytes = 0xbc;
constexpr int32_t kItemCapacity = 2000;
constexpr reg32 kNextMenuField = 0x40;
// The item made last (-1 for none), the menus' string pool and how much of
// it is used (64K), the screens there are, the path of the .MNU read for
// the error messages.
constexpr Var<int32_t> kLastItem{0x660bac};
constexpr Var<reg32> kPoolUsed{0x660ba8};
constexpr Var<int32_t> kMenuCount{0x660bb0};
constexpr Var<reg32> kMenuPath{0x55921c};
constexpr reg32 kPool = 0x5f40d0, kPoolSize = 0x10000;
// The names of the language's lines of text, for the text values.
constexpr Table<reg32> kTextNames{0x561c44};
constexpr int32_t kTextNameCount = 0x789;
// The C library's character classes (Watcom's _IsTable, indexed by c + 1).
constexpr reg32 kCharClasses = 0x564ef0;
constexpr uint8_t kDigit = 0x20;
// The widget a keyword names every widget with, a button's, a keyword that
// takes a screen to load.
constexpr reg32 kAnyWidget = 1, kButton = 5;
// The zeros the .MNU line buffer is copied from in the code.
constexpr reg32 kLineTemplate = 0x4489f0, kLineBytes = 0x502, kLineWords = 0x140;

// The game's strings: the codelinks' separators, "%f", "kText", "kMenu", the
// menus' folder, "%s.mnu", "rb", "nextmenu", ".mnu", the folder's ending.
constexpr reg32 kSeparators = 0x5389e8, kFloatFormat = 0x538f70, kTextPrefix = 0x538f74, kMenuPrefix = 0x538f7c;
constexpr reg32 kMenusDirectory = 0x538f84, kMenuFileFormat = 0x538f94, kReadBinary = 0x538f9c;
constexpr reg32 kNextMenu = 0x538fa0, kMenuExtension = 0x538fac, kBackslash = 0x539600;

/* ---- What they call ---- */

// The port's own, as the game's code calls it.
constexpr reg32 kLanguageText = 0x4d1850;  // port_text's languageText, by its address
constexpr reg32 kMenuEdited = 0x442d00;  // port_buttons's menuEdited, by its address
// What is not rewritten: by address.
constexpr reg32 kStrcmp = 0x4ee310, kStricmp = 0x4e9ec0, kStrstr = 0x4e0820, kStrtok = 0x4eff60;
constexpr reg32 kAtoi = 0x4f0430, kAtol = 0x4f0490, kSscanf = 0x4f0548, kSprintf = 0x4df690;
constexpr reg32 kMemset = 0x4e0640, kFopen = 0x4edfe8, kFgets = 0x4f0570, kFclose = 0x4ee100;
constexpr reg32 kAccess = 0x4f05f0, kKeyWait = 0x4f0120;

/* ---- Small things ---- */

reg32 word(App* app, reg32 at)
{
    return app->getMemory<reg32>(at);
}

void setWord(App* app, reg32 at, reg32 value)
{
    app->getMemory<reg32>(at) = value;
}

int16_t half(App* app, reg32 at)
{
    return app->getMemory<int16_t>(at);
}

void setHalf(App* app, reg32 at, int16_t value)
{
    app->getMemory<int16_t>(at) = value;
}

uint8_t byteAt(App* app, reg32 at)
{
    return app->getMemory<uint8_t>(at);
}

void setByte(App* app, reg32 at, uint8_t value)
{
    app->getMemory<uint8_t>(at) = value;
}

float floatAt(App* app, reg32 at)
{
    return app->getMemory<float>(at);
}

/* The game's string functions, inline: how long a string is, one copied (two
 * bytes a turn, the same bytes as strcpy), one appended. */
reg32 length(App* app, reg32 s)
{
    reg32 n = 0;
    while (byteAt(app, s + n) != 0)
        ++n;
    return n;
}

void copyString(App* app, reg32 to, reg32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = byteAt(app, from);
        setByte(app, to, c);
        if (c == 0)
            break;
    }
}

void appendString(App* app, reg32 to, reg32 from)
{
    copyString(app, to + length(app, to), from);
}

/* rep movsd / rep movsb of `bytes` bytes, forwards. */
void copyBlock(App* app, reg32 to, reg32 from, reg32 bytes)
{
    for (reg32 i = 0; i < bytes / 4; ++i)
        setWord(app, to + 4 * i, word(app, from + 4 * i));
    for (reg32 i = bytes & ~3u; i < bytes; ++i)
        setByte(app, to + i, byteAt(app, from + i));
}

/* inc dl, the class table indexed by it, test 0x20: a digit. */
bool isDigit(App* app, uint8_t c)
{
    return (byteAt(app, kCharClasses + uint8_t(c + 1)) & kDigit) != 0;
}

/* The item `index`'s address: ((index * 3 << 4) - index) * 4 on from the
 * table, 32 bits wide. */
reg32 itemAt(int32_t index)
{
    return kItems + reg32(index) * kItemBytes;
}

/* A call into the game's code; false when it stopped. */
struct Caller
{
    App* app;
    CPU& cpu;
    reg32 r = 0;
    bool operator()(reg32 esp, reg32 target, Args a = {}, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, a, stack, r);
    }
    /* A typed core called as C++ (the C library's, EA's key wait, the language's text) from `esp`,
     * where the game's code calls it: its result in r; false when the game stopped in it.  The
     * registers the call by address loaded are not loaded: nothing here reads them after it. */
    template <reg32 Address, auto Core, typename... A>
    bool direct(reg32 esp, A... args)
    {
        cpu.esp = esp;
        const auto result = guest::direct<Address, Core>(app, cpu, args...);
        if (cpu.terminate)
            return false;
        r = valueOf(result);
        return true;
    }
    static reg32 valueOf(reg32 value) { return value; }
    template <class T>
    static reg32 valueOf(const std::optional<T>& value)
    {
        return reg32(*value);
    }
};

/* A rewrite called from here as the game's code calls it: its entry esp is
 * the one under its return address. */
constexpr reg32 entered(reg32 at)
{
    return at - 4;
}

/* ---- The item lookup ---- */

}  // namespace

/* 0x4429c0's: whether a codelink names `name`.  A codelink may hold several
 * names, apart by spaces; it is split with strtok, on a copy in the frame
 * (pushes ecx, esi, edi, ebp, then 0x200 bytes). */
Result codelinkMatched(App* app, CPU& cpu, reg32 esp, reg32 codelink, reg32 name, reg32 ebx)
{
    if (codelink == 0 || name == 0)
        return 0;
    const reg32 copy = esp - 16 - 0x200;
    copyString(app, copy, codelink);
    Caller call{app, cpu};
    // The game's code has ebx and the name in ebx and ecx too, which strtok and strcmp do not read.
    (void)ebx;
    if (!call.direct<kStrtok, watcom::clib::strtok>(copy, copy, kSeparators))
        return {};
    while (call.r != 0)
    {
        const reg32 token = call.r;
        if (!call.direct<kStrcmp, watcom::clib::strcmp>(copy, token, name))
            return {};
        if (call.r == 0)
            return 1;
        if (!call.direct<kStrtok, watcom::clib::strtok>(copy, reg32(0), kSeparators))
            return {};
    }
    return 0;
}

namespace
{

/* 0x442b10's: a screen's handlers, found by its name in the table at
 * 0x555c48; a screen not there keeps the ones it has.  eax: the last
 * handler copied, or what the last compare left (the menu itself for a
 * table with nothing in it). */
Result handlersAssigned(App* app, CPU& cpu, reg32 esp, reg32 menu)
{
    Caller call{app, cpu};
    reg32 eax = menu;
    const reg32 at = esp - 16;
    for (reg32 entry = kHandlerTable.at; word(app, entry) != 0; entry += sizeof(Handlers))
    {
        if (!call.direct<kStricmp, watcom::clib::stricmp>(at, word(app, entry), word(app, menu + 0x14)))
            return {};
        eax = call.r;
        if (eax != 0)
            continue;
        const Handlers& h = record<Handlers>(app, entry);
        Menu& m = record<Menu>(app, menu);
        for (int i = 0; i < 5; ++i)
            m.handlers[i] = h.handlers[i];
        return int32_t(h.handlers[4]);
    }
    return int32_t(eax);
}

/* ---- The parser's tables ---- */

/* 0x448f00's: a key's row in the keyword table, for a widget of `type`: one
 * of its own or one every widget has.  -1 for none. */
Result keywordFound(App* app, CPU& cpu, reg32 esp, reg32 keyword, reg32 type)
{
    Caller call{app, cpu};
    const reg32 at = esp - 20;
    for (reg32 i = 0;; ++i)
    {
        const Keyword& row = kKeywords(app, int32_t(i));
        if (row.widget == 0)
            return -1;
        if (!call.direct<kStrcmp, watcom::clib::strcmp>(at, keyword, row.name))
            return {};
        if (call.r == 0 && (type == row.widget || row.widget == kAnyWidget))
            return int32_t(i);
    }
}

/* 0x448f50's: a string kept in the menus' string pool; 0 when it is full. */
Result stringStored(App* app, reg32 string)
{
    const reg32 used = kPoolUsed(app);
    if (used + length(app, string) + 1 >= kPoolSize)
        return 0;
    const reg32 copy = kPool + used;
    copyString(app, copy, string);
    kPoolUsed.set(app, kPoolUsed(app) + length(app, string) + 1);
    return int32_t(copy);
}

/* 0x448fd0's: a widget's type by its name, -1 when there is none of it. */
Result widgetTyped(App* app, CPU& cpu, reg32 esp, reg32 name)
{
    Caller call{app, cpu};
    const reg32 at = esp - 20;
    for (int32_t i = 0;; ++i)
    {
        const Widget& w = kWidgets(app, i);
        if (w.type == 0)
            return -1;
        if (!call.direct<kStrcmp, watcom::clib::strcmp>(at, name, w.name))
            return {};
        if (call.r == 0)
            return int32_t(w.type);
    }
}

/* 0x449020's: a widget's name by its type, 0 for none.  Nothing calls it. */
Result widgetNamed(App* app, reg32 type)
{
    for (int32_t i = 0;; ++i)
    {
        const Widget& w = kWidgets(app, i);
        if (w.type == 0)
            return 0;
        if (w.type == type)
            return int32_t(w.name);
    }
}

/* 0x449060's: an item made a widget of `type`, from its template; a type not
 * in the registry leaves it as it is.  eax: the item's size when it was
 * made, else the registry row's offset past the last. */
Result itemInitialised(App* app, reg32 index, reg32 type)
{
    int32_t row = 0;
    while (kWidgets(app, row).type != 0 && kWidgets(app, row).type != type)
        ++row;
    if (kWidgets(app, row).type == 0)
        return int32_t(reg32(row) * sizeof(Widget));
    const reg32 from = kWidgets(app, row).templ;
    const reg32 to = itemAt(int32_t(index));
    copyBlock(app, to, from, kItemBytes);
    return int32_t(kItemBytes);
}

/* 0x4490e0's: the widget a key "[name]" makes, -1 for a key that is no
 * section.  The closing bracket is cut off the key. */
Result sectionTyped(App* app, CPU& cpu, reg32 esp, reg32 key)
{
    if (byteAt(app, key) != '[')
        return -1;
    if (byteAt(app, key + length(app, key) - 1) != ']')
        return -1;
    setByte(app, key + length(app, key) - 1, 0);
    return widgetTyped(app, cpu, entered(esp - 16), key + 1);
}

/* ---- The values ---- */

/* 0x449130's, 0x449170's: a number (atoi's or atol's), when every character
 * is a digit; else 0. */
Result numberParsed(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 convert, const Args& in)
{
    Caller call{app, cpu};
    // atol is atoi's code (watcom::clib::atoi), called at its own address.
    (void)in;
    if (!(convert == kAtol ? call.direct<kAtol, watcom::clib::atoi>(esp - 12, text)
                           : call.direct<kAtoi, watcom::clib::atoi>(esp - 12, text)))
        return {};
    for (reg32 c = text; byteAt(app, c) != 0; ++c)
        if (!isDigit(app, byteAt(app, c)))
            return 0;
    return int32_t(call.r);
}

/* 0x4491b0's: a float, into `parsed` (the thunk leaves it on the x87's
 * stack), when sscanf reads one and every character is a digit, '.', or '-';
 * 0.0 otherwise.  eax: sscanf's result if it read none, the float's bits, or
 * where the first other character is. */
Result floatParsed(App* app, CPU& cpu, reg32 esp, reg32 text, const Args& in, float& parsed)
{
    const reg32 value = esp - 16 - 8;  // [ebp - 8]: sscanf's float; [ebp - 4] the one loaded
    const reg32 loaded = value + 4;
    Caller call{app, cpu};
    // By address: sscanf runs the runtime's own code below the words it pushes, which a direct call
    // would put 4 bytes lower than the game's call does.
    if (!call(value, kSscanf, {value, text, in.ebx, in.ecx}, {text, kFloatFormat, value}))
        return {};
    reg32 eax = call.r;
    if (eax == 0)
        setWord(app, loaded, eax);
    else
    {
        eax = text;
        for (;;)
        {
            const uint8_t c = byteAt(app, eax);
            if (c == 0)
            {
                eax = word(app, value);
                setWord(app, loaded, eax);
                break;
            }
            if (isDigit(app, c) || c == '.' || c == '0' || c == '-')
            {
                ++eax;
                continue;
            }
            setWord(app, loaded, 0);
            break;
        }
    }
    parsed = float(Float(floatAt(app, loaded)));  // fld dword
    return int32_t(eax);
}

/* 0x449240's: an item's field set from a key's value, by the key's kind.
 * eax: how many bytes it stored, or the value for a kind there is not. */
Result fieldSet(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 keyword, reg32 value)
{
    const Keyword& row = kKeywords(app, int32_t(keyword));
    const reg32 field = itemAt(int32_t(index)) + (row.field - kItems);
    const reg32 at = esp - 16 - 0x1c;
    const Args none = {};
    switch (row.kind)
    {
    case kByte:
    case kWord:
    case kDword:
    case kLong:
    {
        const Result n = numberParsed(app, cpu, entered(at), value, row.kind == kLong ? kAtol : kAtoi, none);
        if (!n)
            return {};
        const reg32 size = row.kind == kByte ? 1 : row.kind == kWord ? 2 : 4;
        std::memcpy(&app->getMemory<uint8_t>(field), &*n, size);  // little endian
        return int32_t(size);
    }
    case kString:
    {
        const Result s = stringStored(app, value);
        setWord(app, field, reg32(*s));
        return 4;
    }
    case kText:
    {
        // "kText..." or "kMenu...": the language's line of that name; any
        // other value, or a name not found, the string itself.
        const reg32 string = reg32(*stringStored(app, value));
        reg32 line = 0;
        Caller call{app, cpu};
        if (!call.direct<kStrstr, watcom::clib::strstr>(at, string, kTextPrefix))
            return {};
        bool named = call.r == string;
        if (!named)
        {
            if (!call.direct<kStrstr, watcom::clib::strstr>(at, string, kMenuPrefix))
                return {};
            named = call.r == string;
        }
        if (named)
        {
            for (int32_t id = 0; id < kTextNameCount; ++id)
            {
                const reg32 name = kTextNames(app, id);
                if (!call.direct<kStrcmp, watcom::clib::strcmp>(at, string, name))
                    return {};
                if (call.r != 0)
                    continue;
                // edx the name, handed on as the game's code has it there.
                if (!call.direct<0x4d1850, text::lineText>(at, entered(at), id, name))
                    return {};
                line = call.r;
                break;
            }
        }
        setWord(app, field, line != 0 ? line : string);
        return 4;
    }
    case kFloat:
    {
        float f = 0.0f;
        const Result r = floatParsed(app, cpu, entered(at), value, none, f);
        if (!r)
            return {};
        std::memcpy(&app->getMemory<uint8_t>(field), &f, sizeof f);
        return 4;
    }
    }
    return int32_t(value);
}

/* ---- The file ---- */

/* 0x4493b0's: the next "key = value" of a .MNU, or a section's "[key]";
 * blank lines and comments (';') skipped.  0 at the end of the file. */
Result entryRead(App* app, CPU& cpu, reg32 esp, reg32 file, reg32 key, reg32 value, reg32 lineCount)
{
    const reg32 line = esp - 12 - 0x514;  // char[0x502], from the zeros in the code
    const reg32 at = line;
    Caller call{app, cpu};
    for (reg32 i = 0; i < kLineWords; ++i)
        setWord(app, line + 4 * i, word(app, kLineTemplate + 4 * i));
    setHalf(app, line + 4 * kLineWords, half(app, kLineTemplate + 4 * kLineWords));
    for (;;)
    {
        setByte(app, key, 0);
        setByte(app, value, 0);
        if (!call(at, kFgets, {line, 0x501, file, 0}))
            return {};
        if (call.r == 0)
            return 0;
        setWord(app, lineCount, word(app, lineCount) + 1);

        // Line ends, anywhere in the line, cut.
        for (int32_t i = int32_t(length(app, line)) - 1; i >= 0; --i)
        {
            const uint8_t c = byteAt(app, line + reg32(i));
            if (c == '\n' || c == '\r')
                setByte(app, line + reg32(i), 0);
        }
        if (length(app, line) == 0 || byteAt(app, line) == ';')
            continue;

        // The key: up to '=', without its spaces and tabs.
        reg32 index = 0, keyLength = 0;
        for (;; )
        {
            const uint8_t c = byteAt(app, line + index);
            if (c == 0)
                break;
            ++index;
            setByte(app, key + keyLength++, c);
            if (c == '=')
            {
                --keyLength;
                break;
            }
            if (c == ' ' || c == '\t')
                --keyLength;
        }
        setByte(app, key + keyLength, 0);
        if (keyLength == 0)
            continue;
        if (byteAt(app, line + index) == 0 && byteAt(app, key + keyLength - 1) != ']')
        {
            // Neither a section nor a key with a value: no '=', or nothing after it.
            if (!call.direct<kKeyWait, ea::waitKey>(at))
                return {};
            continue;
        }

        // The value: the rest, from its first character that is no space.
        for (;;)
        {
            const uint8_t c = byteAt(app, line + index);
            if (c == 0)
                break;
            ++index;
            if (c != ' ' && c != '\t')
            {
                --index;
                break;
            }
        }
        reg32 valueLength = 0;
        for (;;)
        {
            const uint8_t c = byteAt(app, line + index);
            if (c == 0)
                break;
            setByte(app, value + valueLength, c);
            ++index;
            ++valueLength;
        }
        setByte(app, value + valueLength, 0);
        return 1;
    }
}

/* 0x4529a0's: the path at `path` ended with a '\'. */
Result backslashAppended(App* app, reg32 path)
{
    const reg32 n = length(app, path);
    if (n == 0 || byteAt(app, path + n - 1) != '\\')
        appendString(app, path, kBackslash);
    return 0;
}

/* 0x449990's: a screen's onLoad handler, once. */
Result onLoadCalled(App* app, CPU& cpu, reg32 esp, reg32 menu, const Args& in)
{
    reg32 eax = menu;
    if (half(app, menu + 0x20) != 0)
        return int32_t(eax);
    const reg32 onLoad = word(app, menu);
    if (onLoad != 0)
    {
        Caller call{app, cpu};
        const HeldEbp held(cpu, esp - 8);
        if (!call(esp - 8, onLoad, {menu, menu, in.ebx, in.ecx}))
            return {};
        eax = call.r;
    }
    setHalf(app, menu + 0x20, 1);
    return int32_t(eax);
}

/* 0x4498a0's: whether a screen's .MNU may be written to (access(path, 2)),
 * for a screen marked edited.  What it was for is gone with the editor. */
Result menuFileChecked(App* app, CPU& cpu, reg32 esp, reg32 index)
{
    const reg32 frame = esp - 20 - 0x12c;
    const reg32 path = frame;
    copyString(app, path, kMenusDirectory);
    backslashAppended(app, path);
    appendString(app, path, kMenus(app, int32_t(index)).name);
    appendString(app, path, kMenuExtension);
    Caller call{app, cpu};
    if (!call(frame, kAccess, {path, 2}))
        return {};
    return int32_t(call.r);
}

/* 0x449960's: checkMenuFile for every screen marked edited (0x442d00).  eax
 * as the last call left it; the one it came with for no screen. */
Result menuFilesChecked(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    Caller call{app, cpu};
    const reg32 at = esp - 8;
    for (int32_t i = 0; i < kMenuCount(app); ++i)
    {
        eax = reg32(i);
        if (!call(at, kMenuEdited, {eax}))
            return {};
        eax = call.r;
        if (eax == 0)
            continue;
        const Result checked = menuFileChecked(app, cpu, entered(at), reg32(i));
        if (!checked)
            return {};
        eax = reg32(*checked);
    }
    return int32_t(eax);
}

}  // namespace

/* 0x449510's: the screen `name` loaded -- with every screen its buttons lead
 * to -- unless it is loaded already.  With `reset`, every screen, item and
 * string loaded so far is let go first.  A screen whose .MNU cannot be
 * opened stays, without items. */
Result menusLoaded(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 reset, const Args& in)
{
    // The frame: pushes ebx, ecx, esi, edi, ebp, then 0x60c bytes of the
    // line's value, the key, the path and the line count.
    const reg32 frame = esp - 20 - 0x60c;
    const reg32 value = frame, key = frame + 0x400, path = frame + 0x500, lineCount = frame + 0x600;
    const reg32 at = frame;
    Caller call{app, cpu};

    if (reset != 0)
    {
        kLastItem.set(app, -1);
        kPoolUsed.set(app, 0);
        kMenuCount.set(app, 0);
        for (int32_t i = 0; i < kItemCapacity; ++i)
            setWord(app, itemAt(i), 0);
    }
    else
    {
        int32_t i = 0;
        for (; i < kMenuCount(app); ++i)
        {
            if (!call.direct<kStrcmp, watcom::clib::strcmp>(at, name, kMenus(app, i).name))
                return {};
            if (call.r == 0)
                break;
        }
        if (i < kMenuCount(app))
            return 1;
    }

    // A screen added at the end of the table, its handlers by its name: the
    // name given, or the last item's nextmenu, read after the screen's record
    // is cleared.
    const auto addMenu = [&](reg32 menuName, bool fromItem) -> bool {
        const int32_t count = kMenuCount(app);
        const reg32 menu = kMenus.address(count);
        if (!call.direct<kMemset, watcom::clib::memset>(at, menu, reg32(0), reg32(sizeof(Menu))))
            return false;
        kMenus(app, count).name = fromItem ? word(app, itemAt(kLastItem(app)) + kNextMenuField) : menuName;
        kMenuCount.set(app, count + 1);
        return bool(handlersAssigned(app, cpu, entered(at), menu));
    };
    if (!addMenu(name, false))
        return {};

    // Each screen added, the ones its buttons add included.
    for (int32_t index = kMenuCount(app) - 1; index < kMenuCount(app); ++index)
    {
        const reg32 menu = kMenus.address(index);
        copyString(app, path, kMenusDirectory);
        backslashAppended(app, path);
        if (!call.direct<kSprintf, watcom::clib::sprintf>(at, path + length(app, path), kMenuFileFormat,
                                                          guest::Words{word(app, menu + 0x14)}))
            return {};
        if (!call.direct<kFopen, watcom::clib::fopen>(at, path, kReadBinary))
            return {};
        const reg32 file = call.r;
        if (file == 0)
            continue;
        kMenuPath.set(app, path);
        setHalf(app, menu + 0x18, int16_t(uint16_t(reg32(kLastItem(app))) + 1));
        setWord(app, lineCount, 0);

        for (;;)
        {
            const Result read = entryRead(app, cpu, entered(at), file, key, value, lineCount);
            if (!read)
                return {};
            if (*read == 0)
                break;

            // A section: the next item, a widget of its type.
            const Result type = sectionTyped(app, cpu, entered(at), key);
            if (!type)
                return {};
            if (*type != -1)
            {
                const int32_t last = kLastItem(app);
                if (last >= kItemCapacity)
                    continue;
                kLastItem.set(app, last + 1);
                if (!itemInitialised(app, reg32(last + 1), reg32(*type)))
                    return {};
                continue;
            }

            // A key: the last item's field set.
            const int32_t last = kLastItem(app);
            const Result keyword = keywordFound(app, cpu, entered(at), key, word(app, itemAt(last)));
            if (!keyword)
                return {};
            if (*keyword == -1)
                continue;
            if (!fieldSet(app, cpu, entered(at), reg32(last), reg32(*keyword), value))
                return {};

            // A button's nextmenu: that screen added, unless it is there.
            const Keyword& row = kKeywords(app, *keyword);
            if (row.widget != kButton)
                continue;
            if (!call.direct<kStrcmp, watcom::clib::strcmp>(at, row.name, kNextMenu))
                return {};
            if (call.r != 0)
                continue;
            if (byteAt(app, word(app, itemAt(kLastItem(app)) + kNextMenuField)) == 0)
                continue;
            int32_t known = 0;
            for (; known < kMenuCount(app); ++known)
            {
                if (!call.direct<kStrcmp, watcom::clib::strcmp>(
                        at, word(app, itemAt(kLastItem(app)) + kNextMenuField), kMenus(app, known).name))
                    return {};
                if (call.r == 0)
                    break;
            }
            if (known != kMenuCount(app))
                continue;
            if (!addMenu(0, true))
                return {};
        }
        if (!call.direct<kFclose, watcom::clib::fclose>(at, file))
            return {};
        // The screen's end: an item made of type 0, which leaves it as it is.
        const int32_t end = kLastItem(app) + 1;
        kLastItem.set(app, end);
        if (!itemInitialised(app, reg32(end), 0))
            return {};
    }

    for (int32_t i = 0; i < kMenuCount(app); ++i)
        if (!onLoadCalled(app, cpu, entered(at), kMenus.address(i), in))
            return {};
    return 1;
}

/* ---- The thunks ---- */

namespace
{

Args regsOf(const Entry& e)
{
    return {e.eax, e.edx, e.ebx, e.ecx};
}

}  // namespace

/* 0x4429c0: whether the codelink at eax names the name at edx. */
x86::reg32 codelinkMatches(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, codelinkMatched(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x442b10: the screen at eax given its handlers by its name. */
x86::reg32 assignHandlers(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, handlersAssigned(app, cpu, e.esp, e.eax));
}

/* 0x448f00: the key at eax's row in the keyword table for a widget of type edx. */
x86::reg32 keywordIndex(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, keywordFound(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x448f50: the string at eax kept in the string pool. */
x86::reg32 storeString(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(*stringStored(app, e.eax)));
}

/* 0x448fd0: the type of the widget named eax. */
x86::reg32 widgetType(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, widgetTyped(app, cpu, e.esp, e.eax));
}

/* 0x449020: the name of the widget of type eax. */
x86::reg32 widgetName(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(*widgetNamed(app, e.eax)));
}

/* 0x449060: item eax made a widget of type edx. */
x86::reg32 initItem(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(*itemInitialised(app, e.eax, e.edx)));
}

/* 0x4490e0: the widget the section key at eax makes. */
x86::reg32 sectionType(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, sectionTyped(app, cpu, e.esp, e.eax));
}

/* 0x449130: the number at eax, int-wide. */
x86::reg32 parseInt(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, numberParsed(app, cpu, e.esp, e.eax, kAtoi, regsOf(e)));
}

/* 0x449170: the number at eax, long-wide. */
x86::reg32 parseLong(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, numberParsed(app, cpu, e.esp, e.eax, kAtol, regsOf(e)));
}

/* 0x4491b0: the float at eax, onto the x87's stack. */
x86::reg32 parseFloat(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    float parsed = 0.0f;
    const Result r = floatParsed(app, cpu, e.esp, e.eax, regsOf(e), parsed);
    if (r)
    {
        args.floatSet = true;
        args.floatResult = double(Float(parsed));
    }
    return e.finish(cpu, args, r);
}

/* 0x449240: item eax's field for keyword edx set from the value at ebx. */
x86::reg32 setField(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, fieldSet(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x4493b0: the next key and value of the file eax into edx and ebx, the lines
 * counted at ecx. */
x86::reg32 readEntry(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, entryRead(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

/* 0x449510: the screen named eax loaded, edx to start over. */
x86::reg32 loadMenus(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, menusLoaded(app, cpu, e.esp, e.eax, e.edx, regsOf(e)));
}

/* 0x4498a0: whether screen eax's file may be written to. */
x86::reg32 checkMenuFile(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, menuFileChecked(app, cpu, e.esp, e.eax));
}

/* 0x449960: every edited screen's file checked. */
x86::reg32 checkMenuFiles(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, menuFilesChecked(app, cpu, e.esp, e.eax));
}

/* 0x449990: the screen at eax's onLoad handler called, once. */
x86::reg32 callOnLoad(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, onLoadCalled(app, cpu, e.esp, e.eax, regsOf(e)));
}

reg32 backslashEnded(App* app, reg32 path)
{
    backslashAppended(app, path);
    return 0;
}

/* 0x4529a0: the path at eax ended with a '\'. */
x86::reg32 appendBackslash(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, pathBackslashed(app, e.eax));
}

/* 0x4529a0's core: the path at `path` ended with a '\'; 0. */
reg32 pathBackslashed(App* app, reg32 path)
{
    return reg32(*backslashAppended(app, path));
}

}  // namespace menus
}  // namespace port
}  // namespace nfs3hp
