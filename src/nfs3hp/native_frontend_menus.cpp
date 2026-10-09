#include "native_frontend.h"
#include <cstring>
#include <iterator>

/* The menu files and the screens they make: MENUS/<name>.MNU read into the
 * table of screens (0x6040d0) and the table of items (0x604ee0), and an item
 * found again by the name code knows it by.
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
 * The section names the widget (the registry at 0x557a84: title, text, image,
 * button, cascade...), whose template the new item starts from.  A key is
 * looked up in the parser's table at 0x557c10 -- the widget it belongs to
 * (1 for every widget), its kind, the item's field it fills -- and its value
 * stored there by kind: a number of 1, 2 or 4 bytes, a string kept in the
 * menus' string pool, a line of the language's text, a float.  Spaces and
 * tabs in a key are dropped; a value starts at its first other character.
 * Names are matched as written, case and all -- widgets, keys, screens,
 * codelinks, text names (strcmp) -- but for a screen's handlers, found by
 * its name in any case.  A button's nextmenu loads that screen too, and the
 * ones it leads to, so opening the first screen reads every one reachable
 * from it.
 *
 * Quirks of the original, kept: a number is taken only when every character
 * of it is a digit, so a negative one reads as 0; a line that is neither a
 * section nor a key = value -- a key with nothing after its '=' included --
 * stops the game with an error (sub_4f0120). */
namespace nfs3hp
{

namespace
{

using fe::Guest;

// The screens and their items.
constexpr x86::reg32 kMenus = 0x6040d0;            // menu::kSize each
constexpr x86::reg32 kMenuCount = 0x660bb0;
constexpr x86::reg32 kLastItem = 0x660bac;         // the item made last, -1 for none
constexpr std::int32_t kItemCapacity = 2000;
// The menus' string pool: what a string key stores, 64K at most.
constexpr x86::reg32 kStringPool = 0x5f40d0;
constexpr x86::reg32 kStringPoolUsed = 0x660ba8;
constexpr std::uint32_t kStringPoolSize = 0x10000;
// The path of the .MNU being read, for the error messages.
constexpr x86::reg32 kMenuPath = 0x55921c;

// The widget registry: type, name, the item a new one starts as (12 bytes).
constexpr x86::reg32 kWidgets = 0x557a84;
constexpr x86::reg32 kWidgetSize = 12;
// The keyword table: widget (1 every one), keyword, kind, field (16 bytes).
constexpr x86::reg32 kKeywords = 0x557c10;
constexpr x86::reg32 kKeywordSize = 16;
constexpr std::uint32_t kAnyWidget = 1;
constexpr std::uint32_t kButton = 5;
enum KeywordKind : std::uint32_t
{
    kByte = 0,
    kWord = 1,
    kDword = 2,
    kLong = 3,      // atol where kDword has atoi; the same on this machine
    kString = 4,
    kText = 5,      // a line of the language's text by its name, or the string
    kFloat = 6,
};
// The screens' handlers by name: name, then menu::kOnLoad...kOnFrame (24 bytes).
constexpr x86::reg32 kHandlerTable = 0x555c48;
constexpr x86::reg32 kHandlerEntrySize = 24;
// The names of the language's lines of text, for kText values.
constexpr x86::reg32 kTextNames = 0x561c44;
constexpr std::uint32_t kTextNameCount = 0x789;

// The game's strings the loader hands on.
constexpr fe::GuestString kSeparators{ 0x5389e8, " " };           // between a codelink's names
constexpr fe::GuestString kFloatFormat{ 0x538f70, "%f" };
constexpr fe::GuestString kTextPrefix{ 0x538f74, "kText" };
constexpr fe::GuestString kMenuPrefix{ 0x538f7c, "kMenu" };
constexpr fe::GuestString kMenusDirectory{ 0x538f84, ".\\FeData\\Menus" };
constexpr fe::GuestString kMenuFileFormat{ 0x538f94, "%s.mnu" };
constexpr fe::GuestString kReadBinary{ 0x538f9c, "rb" };
constexpr fe::GuestString kNextMenu{ 0x538fa0, "nextmenu" };
constexpr fe::GuestString kMenuExtension{ 0x538fac, ".mnu" };

// The C library, Watcom's.
constexpr x86::reg32 kStrcmp = 0x4ee310;        // case and all
constexpr x86::reg32 kStrtok = 0x4eff60;
constexpr x86::reg32 kStrstr = 0x4e0820;
constexpr x86::reg32 kAtoi = 0x4f0430;
constexpr x86::reg32 kAtol = 0x4f0490;
constexpr x86::reg32 kSscanf = 0x4f0548;
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kMemset = 0x4e0640;      // eax where, edx the byte, ebx how many
constexpr x86::reg32 kFopen = 0x4edfe8;
constexpr x86::reg32 kFgets = 0x4f0570;       // eax buffer, edx size, ebx file
constexpr x86::reg32 kFclose = 0x4ee100;
constexpr x86::reg32 kAccess = 0x4f05f0;      // eax path, edx mode
constexpr x86::reg32 kStricmp = 0x4e9ec0;       // A-Z as a-z; only the handler lookup uses it
// The C library's character classes (Watcom's _IsTable, indexed by c + 1).
constexpr x86::reg32 kCharClasses = 0x564ef0;
constexpr std::uint8_t kDigit = 0x20;
// The game's own.
constexpr x86::reg32 kAppendBackslash = 0x4529a0;    // a path ended with '\'
constexpr x86::reg32 kFatalError = 0x4f0120;         // a message, then the game waits for good

// The functions of this file, called where they are so each runs as itself.
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;
constexpr x86::reg32 kAssignHandlers = 0x442b10;
constexpr x86::reg32 kKeywordIndex = 0x448f00;
constexpr x86::reg32 kStoreString = 0x448f50;
constexpr x86::reg32 kWidgetType = 0x448fd0;
constexpr x86::reg32 kInitItem = 0x449060;
constexpr x86::reg32 kSectionType = 0x4490e0;
constexpr x86::reg32 kParseInt = 0x449130;
constexpr x86::reg32 kParseLong = 0x449170;
constexpr x86::reg32 kParseFloat = 0x4491b0;
constexpr x86::reg32 kSetField = 0x449240;
constexpr x86::reg32 kReadEntry = 0x4493b0;
constexpr x86::reg32 kCheckMenuFile = 0x4498a0;
constexpr x86::reg32 kCallOnLoad = 0x449990;
constexpr x86::reg32 kMenuEdited = 0x442d00;

x86::reg32 menuAt(std::uint32_t index)
{
    return kMenus + index * fe::menu::kSize;
}

x86::reg32 itemAt(std::uint32_t index)
{
    return fe::item::kTable + index * fe::item::kSize;
}

bool isDigit(const Guest& guest, std::uint8_t c)
{
    return (guest.byte(kCharClasses + std::uint8_t(c + 1)) & kDigit) != 0;
}

/* sub_4429c0: whether a codelink names `name`.  A codelink may hold several
 * names, apart by spaces; it is split with strtok, on a copy in the frame. */
x86::reg32 codelinkMatches(Guest& guest, x86::reg32 codelink, x86::reg32 name)
{
    if (codelink == 0 || name == 0)
        return 0;
    const x86::reg32 copy = guest.frame();  // char[0x200]
    guest.copyString(copy, codelink);
    for (x86::reg32 token = guest.call(kStrtok, fe::args({ copy, kSeparators.address })); token != 0;
         token = guest.call(kStrtok, fe::args({ 0, kSeparators.address })))
    {
        if (guest.call(kStrcmp, fe::args({ token, name })) == 0)
            return 1;
    }
    return 0;
}

/* sub_442a40: a screen's item by its codelink, 0 when it has none: its items
 * from its first to the end of the screen, an item of type 0. */
x86::reg32 findItem(Guest& guest, x86::reg32 menu, x86::reg32 name)
{
    const std::int16_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    for (x86::reg32 index = x86::reg32(std::int32_t(first));; ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + fe::item::kType) == 0)
            return 0;
        const x86::reg32 codelink = guest.dword(item + fe::item::kCodelink);
        if (codelink != 0 && guest.call(kCodelinkMatches, fe::args({ codelink, name })) != 0)
            return item;
    }
}

/* sub_442b10: a screen's handlers, found by its name in the table at
 * 0x555c48; a screen not there keeps the ones it has. */
x86::reg32 assignHandlers(Guest& guest, x86::reg32 menu)
{
    for (x86::reg32 entry = kHandlerTable; guest.dword(entry) != 0; entry += kHandlerEntrySize)
    {
        if (guest.call(kStricmp, fe::args({ guest.dword(entry), guest.dword(menu + fe::menu::kName) })) == 0)
        {
            for (x86::reg32 slot = 0; slot < 5; ++slot)
                guest.setDword(menu + 4 * slot, guest.dword(entry + 4 + 4 * slot));
            return guest.dword(menu + fe::menu::kOnFrame);  // the last one copied, in eax
        }
    }
    return guest.eax();  // what the last compare left
}

/* sub_448f00: a key's row in the keyword table, for a widget of `type`: one
 * of its own or one every widget has.  -1 for none. */
x86::reg32 keywordIndex(Guest& guest, x86::reg32 keyword, std::uint32_t type)
{
    for (std::uint32_t index = 0;; ++index)
    {
        const x86::reg32 row = kKeywords + index * kKeywordSize;
        const std::uint32_t widget = guest.dword(row);
        if (widget == 0)
            return x86::reg32(-1);
        if (guest.call(kStrcmp, fe::args({ keyword, guest.dword(row + 4) })) == 0
            && (guest.dword(row) == type || guest.dword(row) == kAnyWidget))
            return index;
    }
}

/* sub_448f50: a string kept in the menus' string pool; 0 when it is full. */
x86::reg32 storeString(Guest& guest, x86::reg32 string)
{
    const std::uint32_t used = guest.dword(kStringPoolUsed);
    if (used + guest.length(string) + 1 >= kStringPoolSize)
        return 0;
    const x86::reg32 copy = kStringPool + used;
    guest.copyString(copy, string);
    guest.setDword(kStringPoolUsed, guest.dword(kStringPoolUsed) + guest.length(string) + 1);
    return copy;
}

/* sub_448fd0: a widget's type by its name, -1 when there is none of it. */
x86::reg32 widgetType(Guest& guest, x86::reg32 name)
{
    for (x86::reg32 entry = kWidgets;; entry += kWidgetSize)
    {
        if (guest.dword(entry) == 0)
            return x86::reg32(-1);
        if (guest.call(kStrcmp, fe::args({ name, guest.dword(entry + 4) })) == 0)
            return guest.dword(entry);
    }
}

/* sub_449020: a widget's name by its type, 0 for none.  Nothing calls it. */
x86::reg32 widgetName(Guest& guest, std::uint32_t type)
{
    for (x86::reg32 entry = kWidgets; guest.dword(entry) != 0; entry += kWidgetSize)
    {
        if (guest.dword(entry) == type)
            return guest.dword(entry + 4);
    }
    return 0;
}

/* sub_449060: an item made a widget of `type`, from its template; a type not
 * in the registry leaves it as it is. */
x86::reg32 initItem(Guest& guest, std::uint32_t index, std::uint32_t type)
{
    x86::reg32 entry = kWidgets;
    while (guest.dword(entry) != 0 && guest.dword(entry) != type)
        entry += kWidgetSize;
    if (guest.dword(entry) != 0)
    {
        const x86::reg32 from = guest.dword(entry + 8);
        const x86::reg32 to = itemAt(index);
        for (x86::reg32 i = 0; i < fe::item::kSize; i += 4)
            guest.setDword(to + i, guest.dword(from + i));
    }
    return guest.eax();
}

/* sub_4490e0: the widget a key "[name]" makes, -1 for a key that is no
 * section.  The closing bracket is cut off the key. */
x86::reg32 sectionType(Guest& guest, x86::reg32 key)
{
    if (guest.byte(key) != '[')
        return x86::reg32(-1);
    const x86::reg32 last = key + guest.length(key) - 1;
    if (guest.byte(last) != ']')
        return x86::reg32(-1);
    guest.setByte(last, 0);
    return guest.call(kWidgetType, fe::args({ key + 1 }));
}

/* sub_449130, sub_449170: a number, when every character is a digit. */
x86::reg32 parseNumber(Guest& guest, x86::reg32 text, x86::reg32 convert)
{
    const x86::reg32 value = guest.call(convert, fe::args({ text }));
    for (x86::reg32 c = text; guest.byte(c) != 0; ++c)
    {
        if (!isDigit(guest, guest.byte(c)))
            return 0;
    }
    return value;
}

/* sub_4491b0: a float, left on the x87 stack, when sscanf reads one and
 * every character is a digit, '.', or '-'; 0.0 otherwise. */
x86::reg32 parseFloat(Guest& guest, x86::reg32 text)
{
    const x86::reg32 value = guest.frame();  // float, at [ebp - 8]
    float result = 0.0f;
    if (guest.call(kSscanf, fe::args({}, { text, kFloatFormat.address, value })) != 0)
    {
        // '0' tested apart from the digits too, as the original does.
        x86::reg32 c = text;
        while (guest.byte(c) != 0
               && (isDigit(guest, guest.byte(c)) || guest.byte(c) == '.' || guest.byte(c) == '0'
                   || guest.byte(c) == '-'))
            ++c;
        if (guest.byte(c) == 0)
        {
            const std::uint32_t bits = guest.dword(value);
            static_assert(sizeof bits == sizeof result, "a float is 32 bits");
            std::memcpy(&result, &bits, sizeof result);
        }
    }
    guest.pushFloat(result);
    return guest.eax();
}

/* sub_449240: an item's field set from a key's value, by the key's kind. */
x86::reg32 setField(Guest& guest, std::uint32_t index, std::uint32_t keyword, x86::reg32 value)
{
    const x86::reg32 row = kKeywords + keyword * kKeywordSize;
    const x86::reg32 field = itemAt(index) + (guest.dword(row + 12) - fe::item::kTable);
    switch (guest.dword(row + 8))
    {
    case kByte:
        guest.setByte(field, std::uint8_t(guest.call(kParseInt, fe::args({ value }))));
        break;
    case kWord:
        guest.setWord(field, std::uint16_t(guest.call(kParseInt, fe::args({ value }))));
        break;
    case kDword:
        guest.setDword(field, guest.call(kParseInt, fe::args({ value })));
        break;
    case kLong:
        guest.setDword(field, guest.call(kParseLong, fe::args({ value })));
        break;
    case kString:
        guest.setDword(field, guest.call(kStoreString, fe::args({ value })));
        break;
    case kText:
    {
        // "kText..." or "kMenu...": the language's line of that name; any
        // other value, or a name not found, the string itself.
        const x86::reg32 string = guest.call(kStoreString, fe::args({ value }));
        x86::reg32 text = 0;
        if (guest.call(kStrstr, fe::args({ string, kTextPrefix.address })) == string
            || guest.call(kStrstr, fe::args({ string, kMenuPrefix.address })) == string)
        {
            for (std::uint32_t id = 0; id < kTextNameCount; ++id)
            {
                if (guest.call(kStrcmp, fe::args({ string, guest.dword(kTextNames + 4 * id) })) == 0)
                {
                    text = fe::text(guest, id);
                    break;
                }
            }
        }
        guest.setDword(field, text != 0 ? text : string);
        break;
    }
    case kFloat:
    {
        guest.call(kParseFloat, fe::args({ value }));
        const float f = guest.popFloat();
        std::uint32_t bits;
        std::memcpy(&bits, &f, sizeof bits);
        guest.setDword(field, bits);
        break;
    }
    }
    return guest.eax();
}

/* sub_4493b0: the next "key = value" of a .MNU, or a section's "[key]";
 * blank lines and comments (';') skipped.  0 at the end of the file. */
x86::reg32 readEntry(Guest& guest, x86::reg32 file, x86::reg32 key, x86::reg32 value, x86::reg32 lineCount)
{
    const x86::reg32 line = guest.frame();  // char[0x502]
    for (;;)
    {
        guest.setByte(key, 0);
        guest.setByte(value, 0);
        if (guest.call(kFgets, fe::args({ line, 0x501, file })) == 0)
            return 0;
        guest.setDword(lineCount, guest.dword(lineCount) + 1);

        // Line ends, anywhere in the line, cut.
        for (std::int32_t i = std::int32_t(guest.length(line)) - 1; i >= 0; --i)
        {
            const std::uint8_t c = guest.byte(line + x86::reg32(i));
            if (c == '\n' || c == '\r')
                guest.setByte(line + x86::reg32(i), 0);
        }
        if (guest.length(line) == 0 || guest.byte(line) == ';')
            continue;

        // The key: up to '=', without its spaces and tabs.
        x86::reg32 at = line;
        std::uint32_t keyLength = 0;
        while (const std::uint8_t c = guest.byte(at))
        {
            ++at;
            if (c == '=')
                break;
            if (c != ' ' && c != '\t')
                guest.setByte(key + keyLength++, c);
        }
        guest.setByte(key + keyLength, 0);
        if (keyLength == 0)
            continue;
        if (guest.byte(at) == 0 && guest.byte(key + keyLength - 1) != ']')
        {
            // Neither a section nor a key with a value: no '=', or nothing after it.
            guest.call(kFatalError);
            continue;
        }

        // The value: the rest, from its first character that is no space.
        while (guest.byte(at) == ' ' || guest.byte(at) == '\t')
            ++at;
        guest.copyString(value, at);
        return 1;
    }
}

/* sub_449510: the screen `name` loaded -- with every screen its buttons lead
 * to -- unless it is loaded already.  With `reset`, every screen, item and
 * string loaded so far is let go first.  A screen whose .MNU cannot be
 * opened stays, without items. */
x86::reg32 loadMenus(Guest& guest, x86::reg32 name, x86::reg32 reset)
{
    // The frame as the generated function lays it out.
    const x86::reg32 valueBuffer = guest.frame();          // char[0x400], [ebp - 0x60c]
    const x86::reg32 keyBuffer = guest.frame() + 0x400;    // char[0x100], [ebp - 0x20c]
    const x86::reg32 path = guest.frame() + 0x500;         // char[0x100], [ebp - 0x10c]
    const x86::reg32 lineCount = guest.frame() + 0x600;    // [ebp - 0xc]

    if (reset != 0)
    {
        guest.setDword(kLastItem, x86::reg32(-1));
        guest.setDword(kStringPoolUsed, 0);
        guest.setDword(kMenuCount, 0);
        for (std::int32_t index = 0; index < kItemCapacity; ++index)
            guest.setDword(itemAt(index) + fe::item::kType, 0);
    }
    else
    {
        for (std::uint32_t index = 0; index < guest.dword(kMenuCount); ++index)
        {
            if (guest.call(kStrcmp, fe::args({ name, guest.dword(menuAt(index) + fe::menu::kName) })) == 0)
                return 1;
        }
    }

    // A screen added at the end of the table, its handlers by its name.
    auto addMenu = [&](x86::reg32 menuName) {
        const std::uint32_t count = guest.dword(kMenuCount);
        guest.call(kMemset, fe::args({ menuAt(count), 0, fe::menu::kSize }));
        guest.setDword(menuAt(count) + fe::menu::kName, menuName);
        guest.setDword(kMenuCount, count + 1);
        guest.call(kAssignHandlers, fe::args({ menuAt(count) }));
    };
    addMenu(name);

    // Each screen added, the ones its buttons add included.
    for (std::uint32_t index = guest.dword(kMenuCount) - 1; index < guest.dword(kMenuCount); ++index)
    {
        const x86::reg32 menu = menuAt(index);
        guest.copyString(path, kMenusDirectory.address);
        guest.call(kAppendBackslash, fe::args({ path }));
        guest.call(kSprintf, fe::args({}, { path + guest.length(path), kMenuFileFormat.address,
                                            guest.dword(menu + fe::menu::kName) }));
        const x86::reg32 file = guest.call(kFopen, fe::args({ path, kReadBinary.address }));
        if (file == 0)
            continue;
        guest.setDword(kMenuPath, path);
        guest.setWord(menu + fe::menu::kFirstItem, std::uint16_t(guest.word(kLastItem) + 1));
        guest.setDword(lineCount, 0);

        while (guest.call(kReadEntry, fe::args({ file, keyBuffer, valueBuffer, lineCount })) != 0)
        {
            // A section: the next item, a widget of its type.
            const x86::reg32 type = guest.call(kSectionType, fe::args({ keyBuffer }));
            if (type != x86::reg32(-1))
            {
                const x86::reg32 last = guest.dword(kLastItem);
                if (std::int32_t(last) >= kItemCapacity)
                    continue;
                guest.setDword(kLastItem, last + 1);
                guest.call(kInitItem, fe::args({ last + 1, type }));
                continue;
            }

            // A key: the last item's field set.
            const x86::reg32 last = guest.dword(kLastItem);
            const x86::reg32 keyword
                = guest.call(kKeywordIndex, fe::args({ keyBuffer, guest.dword(itemAt(last) + fe::item::kType) }));
            if (keyword == x86::reg32(-1))
                continue;
            guest.call(kSetField, fe::args({ last, keyword, valueBuffer }));

            // A button's nextmenu: that screen added, unless it is there.
            const x86::reg32 row = kKeywords + keyword * kKeywordSize;
            if (guest.dword(row) != kButton
                || guest.call(kStrcmp, fe::args({ guest.dword(row + 4), kNextMenu.address })) != 0)
                continue;
            const x86::reg32 next = guest.dword(itemAt(guest.dword(kLastItem)) + fe::item::kNextMenu);
            if (guest.byte(next) == 0)
                continue;
            std::uint32_t known = 0;
            while (known < guest.dword(kMenuCount)
                   && guest.call(kStrcmp, fe::args({ guest.dword(itemAt(guest.dword(kLastItem)) + fe::item::kNextMenu),
                                                      guest.dword(menuAt(known) + fe::menu::kName) }))
                          != 0)
                ++known;
            if (known != guest.dword(kMenuCount))
                continue;
            addMenu(guest.dword(itemAt(guest.dword(kLastItem)) + fe::item::kNextMenu));
        }
        guest.call(kFclose, fe::args({ file }));
        // The screen's end: an item made of type 0, which leaves it as it is.
        guest.setDword(kLastItem, guest.dword(kLastItem) + 1);
        guest.call(kInitItem, fe::args({ guest.dword(kLastItem), 0 }));
    }

    for (std::uint32_t index = 0; index < guest.dword(kMenuCount); ++index)
        guest.call(kCallOnLoad, fe::args({ menuAt(index) }));
    return 1;
}

/* sub_4498a0: whether a screen's .MNU may be written to (access(path, 2)),
 * for a screen marked edited.  What it was for is gone with the editor. */
x86::reg32 checkMenuFile(Guest& guest, std::uint32_t index)
{
    const x86::reg32 path = guest.frame();  // char[0x12c]
    guest.copyString(path, kMenusDirectory.address);
    guest.call(kAppendBackslash, fe::args({ path }));
    guest.copyString(path + guest.length(path), guest.dword(menuAt(index) + fe::menu::kName));
    guest.copyString(path + guest.length(path), kMenuExtension.address);
    guest.call(kAccess, fe::args({ path, 2 }));
    return guest.eax();
}

/* sub_449960: checkMenuFile for every screen marked edited (sub_442d00). */
x86::reg32 checkMenuFiles(Guest& guest)
{
    for (std::uint32_t index = 0; index < guest.dword(kMenuCount); ++index)
    {
        if (guest.call(kMenuEdited, fe::args({ index })) != 0)
            guest.call(kCheckMenuFile, fe::args({ index }));
    }
    return guest.eax();
}

/* sub_449990: a screen's onLoad handler, once. */
x86::reg32 callOnLoad(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kLoaded = 0x20;  // int16
    if (guest.word(menu + kLoaded) != 0)
        return guest.eax();
    const x86::reg32 onLoad = guest.dword(menu + fe::menu::kOnLoad);
    if (onLoad != 0)
        guest.call(onLoad, fe::args({ menu }));
    guest.setWord(menu + kLoaded, 1);
    return guest.eax();
}

const fe::GuestString kCodelinkStrings[] = { kSeparators };
const fe::GuestString kParseFloatStrings[] = { kFloatFormat };
const fe::GuestString kSetFieldStrings[] = { kTextPrefix, kMenuPrefix };
const fe::GuestString kLoadStrings[] = { kMenusDirectory, kMenuFileFormat, kReadBinary, kNextMenu };
const fe::GuestString kCheckStrings[] = { kMenusDirectory, kMenuExtension };

}

/* Each stand-in: the generated function, what its ret pops, its frame (the
 * registers it pushes, its locals), the strings it hands to the game, and the
 * registers it restores. */
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::kRestoreEdi;

bool menuCodelinkMatches(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4429c0", 0, 16 + 0x200, kCodelinkStrings, std::size(kCodelinkStrings),
                          kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 codelink = cpu.eax, name = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return codelinkMatches(guest, codelink, name); });
}

bool menuFindItem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442a40", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 menu = cpu.eax, name = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return findItem(guest, menu, name); });
}

bool menuAssignHandlers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442b10", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return assignHandlers(guest, menu); });
}

bool menuKeywordIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448f00", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 keyword = cpu.eax, type = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return keywordIndex(guest, keyword, type); });
}

bool menuStoreString(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448f50", 0, 24, nullptr, 0 };
    const x86::reg32 string = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return storeString(guest, string); });
}

bool menuWidgetType(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448fd0", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 name = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return widgetType(guest, name); });
}

bool menuWidgetName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449020", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 type = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return widgetName(guest, type); });
}

bool menuInitItem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449060", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 index = cpu.eax, type = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return initItem(guest, index, type); });
}

bool menuSectionType(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4490e0", 0, 16, nullptr, 0, kRestoreEcx | kRestoreEdx | kRestoreEdi };
    const x86::reg32 key = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return sectionType(guest, key); });
}

bool menuParseInt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449130", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const x86::reg32 text = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return parseNumber(guest, text, kAtoi); });
}

bool menuParseLong(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449170", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const x86::reg32 text = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return parseNumber(guest, text, kAtol); });
}

bool menuParseFloat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4491b0", 0, 16 + 8, kParseFloatStrings, std::size(kParseFloatStrings),
                          kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 text = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return parseFloat(guest, text); });
}

bool menuSetField(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449240", 0, 16 + 0x1c, kSetFieldStrings, std::size(kSetFieldStrings),
                          kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 index = cpu.eax, keyword = cpu.edx, value = cpu.ebx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return setField(guest, index, keyword, value); });
}

bool menuReadEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4493b0", 0, 12 + 0x514, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 file = cpu.eax, key = cpu.edx, value = cpu.ebx, lineCount = cpu.ecx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return readEntry(guest, file, key, value, lineCount); });
}

bool menuLoadMenus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449510", 0, 20 + 0x60c, kLoadStrings, std::size(kLoadStrings),
                          kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 name = cpu.eax, reset = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return loadMenus(guest, name, reset); });
}

bool menuCheckMenuFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4498a0", 0, 20 + 0x12c, kCheckStrings, std::size(kCheckStrings),
                          kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 index = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return checkMenuFile(guest, index); });
}

bool menuCheckMenuFiles(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449960", 0, 8, nullptr, 0, kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return checkMenuFiles(guest); });
}

bool menuCallOnLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449990", 0, 8, nullptr, 0, kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return callOnLoad(guest, menu); });
}

}
