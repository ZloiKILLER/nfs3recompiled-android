#include <lib/file.h>
#include <SDL3/SDL.h>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

/* The port's versions of the game's menu files, handed to the game in place of
 * the imported ones (win32::File::setReadSubstitute); the imported data stays
 * as it was.
 *
 * MENUS/MAINSPLT.MNU, the main menu for two players on one machine, as the
 * CD has it, lacks two things MAIN.MNU and later copies of the file have:
 * - the makers' names drifting behind the items, MAIN.MNU's image `nfsl`
 *   (codelink texteffects) -- the same [image] put in after `mask`, where the
 *   later copies have it;
 * - car and opponent lists nine lines long (maxvisitem=9), not seven and five.
 * The rows the items stand in are moved up as the screen opens
 * (native_frontend_main.cpp, onSplitEnter), so the longer lists fit.
 *
 * MENUS/SETUP.MNU, the first screen: its "NFS Web Site" button opens Options
 * instead, the way the race screen's Options button does (nextmenu=options,
 * MAIN.MNU's text for it).  Its codelink goes too, or the screen's code
 * (sub_452f80) would find it by SETUP_NfsWebSite and make it open the web
 * site again; the code passes over a button it does not find.
 *
 * MENUS/ADVGRPH.MNU and PSGRAPH.MNU, the graphics reached from the front end
 * and from the pause menu: no Perspective, which picks a rasteriser this port
 * does not have -- OpenGL interpolates perspective-correct by construction, so
 * the entry only ever stood there grey.  Advanced Graphics also gets gamma,
 * brightness and contrast, sliders of the same kind as Alpha Intensity
 * (port_display.cpp binds them), and then takes the eight rows of the front
 * end's table, the ones Graphics itself stands in, in place of six in the
 * middle of them.  The pause menu's has Alpha Intensity alone, in Perspective's
 * row: the display sliders were taken off it at the user's word (2026-11-13).
 *
 * MENUS/CONNECT.MNU, how the game is connected: IPX named for what it is here,
 * and under TCP/IP's button the TCP/IP port and this phone's address, in the
 * rows of the hidden modem and serial; port_connect.cpp fills them in. */

namespace nfs3hp
{

namespace
{

bool endsWith(const std::string& text, const char* suffix)
{
    const std::string tail(suffix);
    if (text.size() < tail.size())
        return false;
    for (size_t i = 0; i < tail.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(text[text.size() - tail.size() + i])) != tail[i])
            return false;
    return true;
}

std::string lower(std::string text)
{
    for (char& c : text)
        c = char(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

bool readWhole(const std::string& path, std::string& out)
{
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data)
        return false;
    out.assign(static_cast<const char*>(data), size);
    SDL_free(data);
    return true;
}

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    for (size_t at = 0; at <= text.size();)
    {
        size_t end = text.find('\n', at);
        if (end == std::string::npos)
            end = text.size();
        std::string line = text.substr(at, end - at);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
        at = end + 1;
    }
    return lines;
}

std::string joinLines(const std::vector<std::string>& lines, const std::string& eol)
{
    std::string joined;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        joined += lines[i];
        if (i + 1 < lines.size())
            joined += eol;
    }
    return joined;
}

std::string endOfLine(const std::string& text)
{
    return text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
}

/* A section of a .MNU: its "[kind]" line and the lines up to the next. */
using Section = std::vector<std::string>;

std::vector<Section> splitSections(const std::vector<std::string>& lines)
{
    std::vector<Section> sections(1);  // what stands before the first "["
    for (const std::string& line : lines)
    {
        if (!line.empty() && line[0] == '[')
            sections.emplace_back();
        sections.back().push_back(line);
    }
    return sections;
}

std::vector<std::string> joinSections(const std::vector<Section>& sections)
{
    std::vector<std::string> lines;
    for (const Section& section : sections)
        lines.insert(lines.end(), section.begin(), section.end());
    return lines;
}

/* The value of `key` (lower case) in the section, lower case; empty without it. */
std::string valueOf(const Section& section, const std::string& key)
{
    const std::string prefix = key + "=";
    for (const std::string& line : section)
        if (lower(line).rfind(prefix, 0) == 0)
            return lower(line.substr(prefix.size()));
    return std::string();
}

/* `key` set to `value`; false where the section has no such line.  The key keeps
 * the file's spelling: the game matches keys case for case (strcmp, 0x448f00),
 * and "texty" for the file's "textY" would be a key it does not know -- the
 * slider's name then never comes to its row. */
bool setValue(Section& section, const std::string& key, const std::string& value)
{
    const std::string prefix = key + "=";
    for (std::string& line : section)
    {
        if (lower(line).rfind(prefix, 0) == 0)
        {
            line = line.substr(0, prefix.size()) + value;
            return true;
        }
    }
    return false;
}

/* The new sliders: their codelinks (port_display.cpp finds them by these),
 * their names and the step that lands on every 5%, so that the setting that
 * leaves the picture alone can be hit exactly.  Ranges in port_display.cpp. */
struct AdjustmentSlider
{
    const char* codelink;
    const char* title;
    const char* step;
};
constexpr AdjustmentSlider kAdjustmentSliders[3] = {
    { "DISPLAY_GAMMA", "Gamma", "0.033333" },
    { "DISPLAY_BRIGHTNESS", "Brightness", "0.050000" },
    { "DISPLAY_CONTRAST", "Contrast", "0.033333" },
};

/* A new slider out of Alpha Intensity's section, without the labels at the ends
 * of its bar; false where it is not laid out as expected. */
bool makeAdjustmentSlider(Section& slider, const AdjustmentSlider& made)
{
    Section kept;
    for (const std::string& line : slider)
    {
        const std::string key = lower(line);
        if (key.rfind("minlabeltext=", 0) != 0 && key.rfind("maxlabeltext=", 0) != 0)
            kept.push_back(line);
    }
    slider = kept;
    return setValue(slider, "codelink", made.codelink) && setValue(slider, "text", made.title)
        && setValue(slider, "snap", made.step) && setValue(slider, "r0_step", made.step);
}

/* The CD's ADVGRPH.MNU with Perspective out, what stands above it a row up and
 * the sliders in the rows below; empty when it is not laid out as expected. */
std::string fixAdvancedGraphics(const std::string& original)
{
    /* The rows the sliders stand in, and where the plate under a slider's name
     * goes for each: its x and y are those of the row (0x559c6c, 0x559c94). */
    constexpr int kAlphaRow = 4;
    struct Plate { const char* x; const char* y; };
    constexpr Plate kPlates[4] = { { "4", "278" }, { "9", "313" }, { "16", "348" }, { "27", "383" } };

    bool perspective = false, cascade = false, plate = false, slider = false;
    int moved = 0;
    std::vector<Section> out;
    for (const Section& section : splitSections(splitLines(original)))
    {
        const std::string codelink = valueOf(section, "codelink");
        const std::string list = valueOf(section, "listname");
        if (codelink == "graph_persp" || codelink == "graph_persp_cascade")
        {
            (codelink == "graph_persp" ? perspective : cascade) = true;
            continue;
        }
        out.push_back(section);
        Section& kept = out.back();
        if (list == "viewlevel" || list == "zbuffer" || list == "viewangle" || list == "projtex")
        {
            const std::string row = valueOf(kept, "ypos");
            if (row.size() != 1 || row[0] < '1' || row[0] > '4')
                return std::string();
            setValue(kept, "ypos", std::string(1, char(row[0] - 1)));
            ++moved;
        }
        else if (codelink == "sfx_intensity")
        {
            const Section alpha = kept;
            if (!setValue(kept, "ypos", std::to_string(kAlphaRow)))
                return std::string();
            for (int i = 0; i < 3; ++i)
            {
                Section made = alpha;
                if (!makeAdjustmentSlider(made, kAdjustmentSliders[i])
                    || !setValue(made, "ypos", std::to_string(kAlphaRow + 1 + i)))
                    return std::string();
                out.push_back(made);
            }
            slider = true;
        }
        else if (valueOf(kept, "image") == "sbut")
        {
            const Section first = kept;
            if (!setValue(kept, "x", kPlates[0].x) || !setValue(kept, "y", kPlates[0].y))
                return std::string();
            for (int i = 1; i < 4; ++i)
            {
                Section made = first;
                setValue(made, "x", kPlates[i].x);
                setValue(made, "y", kPlates[i].y);
                out.push_back(made);
            }
            plate = true;
        }
    }
    // Each of the four lists has its button and its cascade.
    if (!perspective || !cascade || !plate || !slider || moved != 8)
        return std::string();
    return joinLines(joinSections(out), endOfLine(original));
}

/* The CD's PSGRAPH.MNU with Perspective out and Alpha Intensity, under its own
 * name, in its place; empty when it is not laid out as expected. */
std::string fixPauseGraphics(const std::string& original)
{
    constexpr int kAlphaY = 320;
    bool perspective = false, slider = false;
    std::vector<Section> out;
    for (const Section& section : splitSections(splitLines(original)))
    {
        const std::string codelink = valueOf(section, "codelink");
        if (codelink == "graph_persp")
        {
            perspective = true;
            continue;
        }
        out.push_back(section);
        if (codelink != "sfx_intensity")
            continue;
        if (!setValue(out.back(), "y", std::to_string(kAlphaY))
            || !setValue(out.back(), "texty", std::to_string(kAlphaY)))
            return std::string();
        slider = true;
    }
    if (!perspective || !slider)
        return std::string();
    return joinLines(joinSections(out), endOfLine(original));
}

/* The CD's MAINSPLT.MNU with the two changes; empty when it has them already
 * or is not laid out as expected. */
std::string fixSplitMenu(const std::string& original)
{
    if (lower(original).find("image=nfsl") != std::string::npos)
        return std::string();
    const std::vector<std::string> lines = splitLines(original);

    // Sections: where each starts, and the lists' and the mask's.
    std::vector<std::string> out;
    bool afterMask = false, logoIn = false;
    std::string sectionList;
    std::vector<size_t> maxVisible;  // the lines of the section now open
    auto closeSection = [&]() {
        if (sectionList == "cars" || sectionList == "car2" || sectionList == "opcars")
            for (size_t line : maxVisible)
                out[line] = "maxvisitem=9";
        sectionList.clear();
        maxVisible.clear();
    };
    for (const std::string& line : lines)
    {
        const std::string key = lower(line);
        if (!key.empty() && key[0] == '[')
        {
            closeSection();
            if (afterMask && !logoIn)
            {
                for (const char* logo : { "[image]", "x=236", "y=118", "priority=6", "codelink=texteffects",
                                          "image=nfsl", "fadelevel=0", "" })
                    out.push_back(logo);
                logoIn = true;
            }
        }
        else if (key == "image=mask")
            afterMask = true;
        else if (key.rfind("listname=", 0) == 0)
            sectionList = key.substr(9);
        else if (key.rfind("maxvisitem=", 0) == 0)
            maxVisible.push_back(out.size());
        out.push_back(line);
    }
    closeSection();
    if (!logoIn)
        return std::string();
    return joinLines(out, endOfLine(original));
}

/* The CD's SETUP.MNU with the web site button turned into Options; empty when
 * it has no such button. */
std::string fixSetupMenu(const std::string& original)
{
    std::vector<std::string> lines = splitLines(original);

    // The section the button is in: from the [button] before its codelink to
    // the next section.
    size_t codelink = lines.size();
    for (size_t i = 0; i < lines.size(); ++i)
        if (lower(lines[i]) == "codelink=setup_nfswebsite")
            codelink = i;
    if (codelink == lines.size())
        return std::string();
    size_t end = codelink + 1;
    while (end < lines.size() && (lines[end].empty() || lines[end][0] != '['))
        ++end;
    lines[codelink] = "codelink=SETUP_OPTIONS";
    bool text = false;
    for (size_t i = codelink + 1; i < end; ++i)
    {
        if (lower(lines[i]).rfind("text=", 0) == 0)
        {
            lines[i] = "text=kMenuMAINOptions001";
            lines.insert(lines.begin() + std::ptrdiff_t(i) + 1, "nextmenu=options");
            text = true;
            break;
        }
    }
    if (!text)
        return std::string();
    return joinLines(lines, endOfLine(original));
}

/* A line of text of the connection screen; text the place port_connect.cpp
 * writes into, its length the room it has there. */
Section connectText(const char* codelink, const char* text, int x, int y, int colour, int row, int align)
{
    return { "[text]", "x=" + std::to_string(x), "y=" + std::to_string(y), "priority=0",
             std::string("codelink=") + codelink, "postonly=0", "mainonly=0", "EnableForDemo=1", "EnableForE3Demo=1",
             "", std::string("text=") + text, "texttype=0", "color=" + std::to_string(colour), "italic=0",
             "scale=0.000000", "ypos=" + std::to_string(row), "align=" + std::to_string(align), "", "" };
}

/* The CD's CONNECT.MNU with those changes; empty when it is not laid out as
 * expected. */
std::string fixConnectMenu(const std::string& original)
{
    // Rows 4 and 5 of the table at 0x559c6c, 0x559c94; align 3 stands a text beside its row.
    constexpr int kPortRow = 4, kAddressRow = 5, kAddressRowX = 9, kAddressRowY = 313;
    constexpr int kYellow = 0, kBlue = 1, kBesideRow = 3, kPlateText = 11;
    bool ipx = false, tcp = false;
    std::vector<Section> out;
    for (const Section& section : splitSections(splitLines(original)))
    {
        const std::string codelink = valueOf(section, "codelink");
        out.push_back(section);
        if (codelink == "ipx")
        {
            ipx = setValue(out.back(), "text", "Local WiFi");
        }
        else if (codelink == "tcp")
        {
            Section port;
            for (const std::string& line : section)
            {
                const std::string key = lower(line);
                if (key.rfind("help=", 0) != 0 && key.rfind("nextmenu=", 0) != 0)
                    port.push_back(line);
            }
            tcp = setValue(port, "codelink", "NET_PORT") && setValue(port, "text", "TCP Port")
                && setValue(port, "ypos", std::to_string(kPortRow));
            if (!tcp)
                return std::string();
            out.push_back(port);
            out.push_back(connectText("NET_PORT_VALUE", "00000000000000000000", 0, 0, kYellow, kPortRow, kBesideRow));
            out.push_back({ "[image]", "x=" + std::to_string(kAddressRowX), "y=" + std::to_string(kAddressRowY),
                            "priority=6", "postonly=0", "mainonly=0", "EnableForDemo=1", "EnableForE3Demo=1", "",
                            "image=sbut", "fadelevel=95", "borderColor=0", "", "" });
            out.push_back(connectText("NET_ADDRESS_LABEL", "Your IP", kAddressRowX + kPlateText, kAddressRowY, kBlue, 0, 0));
            out.push_back(connectText("NET_ADDRESS_VALUE", "000.000.000.000", 0, 0, kYellow, kAddressRow, kBesideRow));
        }
    }
    if (!ipx || !tcp)
        return std::string();
    return joinLines(joinSections(out), endOfLine(original));
}

std::string cacheDirectory()
{
#ifdef __ANDROID__
    const char* cache = SDL_GetAndroidCachePath();
    return cache ? std::string(cache) + "/" : std::string();
#else
    char* pref = SDL_GetPrefPath("nfs3hp", "port");
    std::string path = pref ? pref : "";
    SDL_free(pref);
    return path;
#endif
}

/* The port's version of the menu file at `path`, made once a run by `fix` from
 * the file the game would have read and kept as `name` in the cache; empty
 * when there is none, and the game reads its own. */
struct MadeMenu
{
    std::string made;
    bool tried = false;
};

std::string portMenu(const std::string& path, const char* name, std::string (*fix)(const std::string&),
                     MadeMenu& state)
{
    if (state.tried)
        return state.made;
    state.tried = true;
    std::string original;
    if (!readWhole(path, original))
        return state.made;
    const std::string fixed = fix(original);
    const std::string directory = cacheDirectory();
    if (fixed.empty() || directory.empty())
        return state.made;
    const std::string copy = directory + lower(name);
    if (!SDL_SaveFile(copy.c_str(), fixed.data(), fixed.size()))
    {
        SDL_Log("[FE] %s: cannot write %s: %s", name, copy.c_str(), SDL_GetError());
        return state.made;
    }
    SDL_Log("[FE] %s: the port's version, %s", name, copy.c_str());
    state.made = copy;
    return state.made;
}

std::string menuData(const std::string& path)
{
    static MadeMenu split, setup, advanced, pause, connect;
    if (endsWith(path, "/fedata/menus/mainsplt.mnu"))
        return portMenu(path, "MAINSPLT.MNU", fixSplitMenu, split);
    if (endsWith(path, "/fedata/menus/setup.mnu"))
        return portMenu(path, "SETUP.MNU", fixSetupMenu, setup);
    if (endsWith(path, "/fedata/menus/advgrph.mnu"))
        return portMenu(path, "ADVGRPH.MNU", fixAdvancedGraphics, advanced);
    if (endsWith(path, "/fedata/menus/psgraph.mnu"))
        return portMenu(path, "PSGRAPH.MNU", fixPauseGraphics, pause);
    if (endsWith(path, "/fedata/menus/connect.mnu"))
        return portMenu(path, "CONNECT.MNU", fixConnectMenu, connect);
    return std::string();
}

}

void installMenuData()
{
    win32::File::setReadSubstitute(menuData);
}

}
