#include <IconsLucide.h>
#include <cstdlib>
#include <imgui.h>
#include <imnodes.h>
#include <json/reader.h>
#include <json/value.h>
#include "common.h"
#include "components.h"
#include "configuration.h"
#include "ui.h"

#define DL(_a, _b) (t.is_dark ? (_a) : (_b))

namespace lcs::ui {
static std::map<std::string, LcsTheme> themes;
static std::vector<const char*> names_light {};
size_t selected_light = 0;
static std::vector<const char*> names_dark {};
size_t selected_dark = 0;

static ImFont* _FONT[FontFlags::FONT_S] = {};

static void _init_fonts(ImGuiIO& io);
static void _init_default_themes(void);
static void _init_themes(Configuration& cfg);
static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT]);

ImFont* get_font(int attributes) { return _FONT[attributes]; }

void set_style(ImGuiIO& io, bool init)
{
    if (init) {
        L_DEBUG("Initializing theme for the first time");
    }
    Configuration& cfg      = get_config();
    cfg.is_applied          = true;
    ImGuiStyle& style       = ImGui::GetStyle();
    ImNodesStyle& nodestyle = ImNodes::GetStyle();
    if (init) {
        _init_fonts(io);
        io.FontDefault = _FONT[REGULAR | NORMAL];
        _init_themes(cfg);
    }
    _set_colors(style.Colors, nodestyle.Colors);

    style.Alpha             = 1.0f;
    style.FrameRounding     = cfg.rounded_corners;
    style.WindowRounding    = cfg.rounded_corners;
    style.ChildRounding     = cfg.rounded_corners;
    style.GrabRounding      = cfg.rounded_corners;
    style.PopupRounding     = cfg.rounded_corners;
    style.ScrollbarRounding = cfg.rounded_corners;
    style.TabRounding       = cfg.rounded_corners;

    style.WindowTitleAlign = ImVec2(0.5, 0.5);
    style.WindowPadding    = ImVec2(10, 15);
    style.FramePadding     = ImVec2(5, 5);
    style.ItemSpacing      = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 2);
    style.IndentSpacing    = 25.0f;
    style.ScrollbarSize    = 10.0f;
    style.GrabMinSize      = 5.0f;

    style.ChildBorderSize         = 1.f;
    style.FrameBorderSize         = 0.f;
    style.PopupBorderSize         = 1.f;
    style.TabBarBorderSize        = 1.f;
    style.WindowBorderSize        = 1.f;
    style.SeparatorTextBorderSize = 0.f;

    io.FontGlobalScale = cfg.scale / 100.f * 2 / 3 /* font downscale factor*/;
    style.ScaleAllSizes(cfg.scale / 100.f);
    if (init) {
        L_DEBUG("Styling is completed.");
    }
}

const char* get_style(size_t idx, bool is_dark)
{
    if (is_dark) {
        return names_dark.at(idx);
    }
    return names_light.at(idx);
}

const LcsTheme& get_theme(const std::string& s)
{
    if (auto t = themes.find(s); t != themes.end()) {
        return t->second;
    }
    lcs_assert(!names_dark.empty() && !names_light.empty());
    return themes.find(names_dark[0])->second;
}

const std::vector<const char*>& get_available_styles(bool is_dark)
{
    return is_dark ? names_dark : names_light;
}

const LcsTheme& get_active_style(void)
{
    Configuration& config = get_config();
    return get_theme(config.preference == Configuration::ALWAYS_LIGHT
            ? config.light_theme
            : config.dark_theme);
}

static void _init_fonts(ImGuiIO& io)
{
    L_DEBUG("Load fonts.");
    // Load the fonts twice the size and scale them back to have clear
    // visuals.
    float fsize = 16.f * 3 / 2;

    ImFontAtlas* atlas = io.Fonts;
    atlas->Clear();
    io.Fonts->AddFontDefault();
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };

    auto dir         = fs::MISC / "font" / "Archivo" / "ttf";
    auto font_lucide = fs::MISC / "font" / "Lucide" / "Lucide.ttf";

    std::string font = (dir / "Archivo-Regular.ttf").string();
    _FONT[SMALL | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), 0.75 * fsize, nullptr);
    _FONT[NORMAL | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr);
    _FONT[LARGE | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), 1.25 * fsize, nullptr);

    font = (dir / "Archivo-Italic.ttf").string();
    _FONT[SMALL | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), 0.75 * fsize, nullptr);
    _FONT[NORMAL | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr);
    _FONT[LARGE | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), 1.25 * fsize, nullptr);

    font                = (dir / "Archivo-Bold.ttf").string();
    _FONT[SMALL | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[NORMAL | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[LARGE | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, atlas->GetGlyphRangesDefault());

    font                         = (dir / "Archivo-BoldItalic.ttf").string();
    _FONT[SMALL | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[NORMAL | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[LARGE | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, atlas->GetGlyphRangesDefault());

    font = (fs::MISC / "font" / "Lucide" / "Lucide.ttf").string();
    _FONT[SMALL | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, icon_ranges);
    _FONT[NORMAL | ICON]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr, icon_ranges);
    _FONT[LARGE | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, icon_ranges);
    _FONT[ULTRA | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 2.5 * fsize, nullptr, icon_ranges);
}

#define CLRU32(...) ImGui::GetColorU32(__VA_ARGS__)

static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT])
{
    const LcsTheme& t = get_active_style();

    clr_gui[ImGuiCol_WindowBg]       = t.bg;
    clr_gui[ImGuiCol_Text]           = t.fg;
    clr_gui[ImGuiCol_TextDisabled]   = v4mul(t.fg, 0.7f);
    clr_gui[ImGuiCol_TextSelectedBg] = v4mul(t.blue, 0.8f);
    clr_gui[ImGuiCol_TextLink]       = t.blue_bright;

    // Headers
    clr_gui[ImGuiCol_MenuBarBg] = v4mul(t.bg, DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_Header]    = v4mul(DL(t.black, t.white), DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_HeaderHovered]
        = v4mul(DL(t.black, t.white), DL(0.7f, 1.f));
    clr_gui[ImGuiCol_HeaderActive]
        = v4mul(DL(t.black, t.white), DL(0.75f, 1.05f));

    // Buttons
    clr_gui[ImGuiCol_Button] = v4mul(DL(t.blue, t.blue_bright), DL(0.5f, 0.9f));
    clr_gui[ImGuiCol_ButtonHovered]
        = v4mul(DL(t.blue, t.blue_bright), DL(0.8f, 1.0f));
    clr_gui[ImGuiCol_ButtonActive]
        = v4mul(DL(t.blue, t.blue_bright), DL(0.8f, 1.1f));
    clr_gui[ImGuiCol_CheckMark] = t.yellow;

    clr_gui[ImGuiCol_DockingPreview] = v4mul(t.blue_bright, 1.f, 0.5f);
    clr_gui[ImGuiCol_DockingEmptyBg] = v4mul(t.blue_bright, 1.f, 0.5f);

    // Frame BG
    clr_gui[ImGuiCol_FrameBg]        = v4mul(t.bg, DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_FrameBgHovered] = v4mul(t.bg, DL(1.7f, 1.2f));
    clr_gui[ImGuiCol_FrameBgActive]  = v4mul(t.bg, DL(1.7f, 1.2f));
    clr_gui[ImGuiCol_ChildBg]        = v4mul(t.bg, DL(0.75f, 0.9f));
    clr_gui[ImGuiCol_PopupBg]        = v4mul(t.bg, DL(0.75f, 0.9f));
    clr_gui[ImGuiCol_ModalWindowDimBg]
        = v4mul(DL(t.black, t.white), 0.5f, 0.5f);

    // Tabs
    clr_gui[ImGuiCol_Tab]                = v4mul(DL(t.black, t.white), 0.8f);
    clr_gui[ImGuiCol_TabHovered]         = v4mul(DL(t.black, t.white), 1.f);
    clr_gui[ImGuiCol_TabActive]          = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TabUnfocused]       = v4mul(DL(t.black, t.white), 0.5f);
    clr_gui[ImGuiCol_TabUnfocusedActive] = v4mul(DL(t.black, t.white), 0.7f);

    // Title
    clr_gui[ImGuiCol_TitleBg]          = v4mul(DL(t.black, t.white), 0.8f);
    clr_gui[ImGuiCol_TitleBgActive]    = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TitleBgCollapsed] = t.bg;

    clr_gui[ImGuiCol_Border]            = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ResizeGrip]        = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ResizeGripHovered] = t.fg;
    clr_gui[ImGuiCol_ResizeGripActive]  = t.fg;
    clr_gui[ImGuiCol_Separator]         = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_SeparatorHovered]  = t.fg;
    clr_gui[ImGuiCol_SeparatorActive]   = t.fg;
    clr_gui[ImGuiCol_TableBorderStrong] = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TableBorderLight]  = v4mul(DL(t.black, t.white), 0.9f);

    clr_gui[ImGuiCol_ScrollbarBg] = v4mul(DL(t.black, t.white), DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_ScrollbarGrab] = v4mul(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ScrollbarGrabHovered]
        = v4mul(DL(t.black, t.white), DL(0.7f, 1.f));
    clr_gui[ImGuiCol_ScrollbarGrabActive]
        = v4mul(DL(t.black, t.white), DL(0.75f, 1.05f));

    clr_gui[ImGuiCol_TableHeaderBg]
        = v4mul(DL(t.yellow, t.yellow_bright), DL(0.8f, 1.f));
    clr_gui[ImGuiCol_TableRowBg]    = t.bg;
    clr_gui[ImGuiCol_TableRowBgAlt] = v4mul(t.bg, 0.8f);

    clr_gui[ImGuiCol_PlotLines] = v4mul(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotLinesHovered]
        = v4mul(DL(t.green, t.yellow_bright), 1.0f);
    clr_gui[ImGuiCol_PlotHistogram] = v4mul(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotHistogramHovered]
        = v4mul(DL(t.green, t.yellow_bright), 1.0f);
    clr_node[ImNodesCol_GridBackground] = CLRU32(v4mul(t.bg, DL(1.2f, 0.8f)));
    clr_node[ImNodesCol_GridLine]       = CLRU32(v4mul(t.fg, 0.5f));
    //    clr_node[ImNodesCol_GridLinePrimary] = CLRU32(ImGuiCol_FrameBgActive);
    clr_node[ImNodesCol_NodeBackground]         = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_NodeBackgroundHovered]  = CLRU32(v4mul(t.bg, 1.3f));
    clr_node[ImNodesCol_NodeBackgroundSelected] = CLRU32(v4mul(t.bg, 1.5f));

    clr_node[ImNodesCol_NodeOutline] = CLRU32(v4mul(t.fg, 0.5f));

    clr_node[ImNodesCol_TitleBar]         = CLRU32(ImGuiCol_Tab);
    clr_node[ImNodesCol_TitleBarHovered]  = CLRU32(ImGuiCol_TabHovered);
    clr_node[ImNodesCol_TitleBarSelected] = CLRU32(ImGuiCol_TabActive);
    clr_node[ImNodesCol_Link]             = CLRU32(ImGuiCol_Button);
    clr_node[ImNodesCol_LinkHovered]      = CLRU32(ImGuiCol_ButtonHovered);
    clr_node[ImNodesCol_LinkSelected]     = CLRU32(ImGuiCol_ButtonActive);
    clr_node[ImNodesCol_Pin]              = CLRU32(v4mul(t.yellow, 1.0f));
    clr_node[ImNodesCol_PinHovered]  = CLRU32(v4mul(t.yellow_bright, 1.2f));
    clr_node[ImNodesCol_BoxSelector] = CLRU32(ImGuiCol_DockingPreview);
    clr_node[ImNodesCol_BoxSelectorOutline] = CLRU32(ImGuiCol_DockingEmptyBg);

    clr_node[ImNodesCol_MiniMapBackground]        = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapBackgroundHovered] = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapOutline]           = CLRU32(ImGuiCol_FrameBg);
    clr_node[ImNodesCol_MiniMapOutlineHovered]
        = CLRU32(ImGuiCol_FrameBgHovered);

    clr_node[ImNodesCol_MiniMapNodeBackground] = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapNodeBackgroundHovered]
        = CLRU32(v4mul(DL(t.black, t.white), 1.1f));
    clr_node[ImNodesCol_MiniMapNodeBackgroundSelected]
        = CLRU32(v4mul(DL(t.black, t.white), 1.1f));
    //    clr_node[ImNodesCol_MiniMapNodeOutline] =
    //    clr_node[ImNodesCol_MiniMapLink] =
    //    clr_node[ImNodesCol_MiniMapLinkSelected] =
    //  clr_node[ImNodesCol_MiniMapCanvas]        = CLRU32(ImGuiCol_FrameBg);
    //  clr_node[ImNodesCol_MiniMapCanvasOutline] = CLRU32(ImGuiCol_Text);
    //  colors[ImGuiCol_DragDropTarget] = theme.cyan_bright;
    //  colors[ImGuiCol_NavHighlight]   = theme.yellow; // renamed in some
    //  versions colors[ImGuiCol_NavWindowingHighlight] = theme.cyan;
    //  colors[ImGuiCol_NavWindowingDimBg]     = V4MUL(theme.bg, 0.5f);
}

static void _init_themes(Configuration& cfg)
{
    L_DEBUG("Load themes.");
    // Load the fonts twice the size and scale them back to have clear
    _init_default_themes();
    Json::Value v {};
    Json::Reader r;
    std::string data;
    if (fs::read(fs::MISC / "themes.json", data)) {
        lcs_assert(r.parse(data, v));
        lcs_assert(v.isArray());
        for (const auto& element : v) {
            LcsTheme s;
            if (s.from_json(element) != Error::OK) {
                L_WARN("Invalid theme: %s", v.toStyledString().c_str());
            }
            themes.emplace(s.name, s);
        }
    }
    names_light.reserve(themes.size());
    names_dark.reserve(themes.size());
    for (const auto& s : themes) {
        if (s.second.is_dark) {
            names_dark.push_back(s.first.c_str());
        } else {
            names_light.push_back(s.first.c_str());
        }
    }
    if (themes.find(cfg.light_theme) == themes.end()) {
        L_ERROR("Light theme preference %s were not found. Falling back to %s",
            cfg.light_theme.c_str(), names_light[0]);
        cfg.light_theme = names_light[0];
    }
    if (themes.find(cfg.dark_theme) == themes.end()) {
        L_ERROR("Dark theme preference %s were not found. Falling back to %s",
            cfg.dark_theme.c_str(), names_dark[0]);
        cfg.dark_theme = names_dark[0];
    }
}

inline ImVec4 to_imvec4(uint64_t clr)
{
    return ImVec4(((clr >> 16) & 0xFF) / 255.0f, ((clr >> 8) & 0xFF) / 255.0f,
        (clr & 0xFF) / 255.0f, 1.0f);
}

inline ImVec4 to_imvec4(const std::string& hex)
{
    lcs_assert(hex.size() == 7 || hex[0] == '#');
    return to_imvec4(std::strtoul(hex.c_str() + 1, nullptr, 16));
}

Json::Value LcsTheme::to_json() const
{
#define CLRU32(...) ImGui::GetColorU32(__VA_ARGS__)
    Json::Value v;
    v["name"]           = name;
    v["is_dark"]        = is_dark;
    v["bg"]             = "#" + std::to_string(CLRU32(bg));
    v["fg"]             = "#" + std::to_string(CLRU32(fg));
    v["black"]          = "#" + std::to_string(CLRU32(black));
    v["red"]            = "#" + std::to_string(CLRU32(red));
    v["green"]          = "#" + std::to_string(CLRU32(green));
    v["yellow"]         = "#" + std::to_string(CLRU32(yellow));
    v["blue"]           = "#" + std::to_string(CLRU32(blue));
    v["magenta"]        = "#" + std::to_string(CLRU32(magenta));
    v["cyan"]           = "#" + std::to_string(CLRU32(cyan));
    v["white"]          = "#" + std::to_string(CLRU32(white));
    v["black_bright"]   = "#" + std::to_string(CLRU32(black_bright));
    v["red_bright"]     = "#" + std::to_string(CLRU32(red_bright));
    v["green_bright"]   = "#" + std::to_string(CLRU32(green_bright));
    v["yellow_bright"]  = "#" + std::to_string(CLRU32(yellow_bright));
    v["blue_bright"]    = "#" + std::to_string(CLRU32(blue_bright));
    v["magenta_bright"] = "#" + std::to_string(CLRU32(magenta_bright));
    v["cyan_bright"]    = "#" + std::to_string(CLRU32(cyan_bright));
    v["white_bright"]   = "#" + std::to_string(CLRU32(white_bright));
    return v;
}
LCS_ERROR LcsTheme::from_json(const Json::Value& v)
{
    if (!(v["name"].isString() && v["is_dark"].isBool() && v["bg"].isString()
            && v["fg"].isString() && v["black"].isString()
            && v["red"].isString() && v["green"].isString()
            && v["yellow"].isString() && v["blue"].isString()
            && v["magenta"].isString() && v["cyan"].isString()
            && v["white"].isString() && v["black_bright"].isString()
            && v["red_bright"].isString() && v["green_bright"].isString()
            && v["yellow_bright"].isString() && v["blue_bright"].isString()
            && v["magenta_bright"].isString() && v["cyan_bright"].isString()
            && v["white_bright"].isString())) {
        return ERROR(Error::INVALID_JSON);
    }
    name           = v["name"].asString();
    is_dark        = v["is_dark"].asBool();
    bg             = to_imvec4(v["bg"].asString());
    fg             = to_imvec4(v["fg"].asString());
    black          = to_imvec4(v["black"].asString());
    red            = to_imvec4(v["red"].asString());
    green          = to_imvec4(v["green"].asString());
    yellow         = to_imvec4(v["yellow"].asString());
    blue           = to_imvec4(v["blue"].asString());
    magenta        = to_imvec4(v["magenta"].asString());
    cyan           = to_imvec4(v["cyan"].asString());
    white          = to_imvec4(v["white"].asString());
    black_bright   = to_imvec4(v["black_bright"].asString());
    red_bright     = to_imvec4(v["red_bright"].asString());
    green_bright   = to_imvec4(v["green_bright"].asString());
    yellow_bright  = to_imvec4(v["yellow_bright"].asString());
    blue_bright    = to_imvec4(v["blue_bright"].asString());
    magenta_bright = to_imvec4(v["magenta_bright"].asString());
    cyan_bright    = to_imvec4(v["cyan_bright"].asString());
    white_bright   = to_imvec4(v["white_bright"].asString());
    return OK;
}

static void _init_default_themes(void)
{
    LcsTheme light;
    LcsTheme dark;
    // Colors (Ayu Light)
    // Default colors - taken from ayu-colors
    light.name           = "Default (Light)";
    light.is_dark        = false;
    light.bg             = to_imvec4(0xFCFCFC);
    light.fg             = to_imvec4(0x5C6166);
    light.black          = to_imvec4(0x010101);
    light.red            = to_imvec4(0xe7666a);
    light.green          = to_imvec4(0x80ab24);
    light.yellow         = to_imvec4(0xeba54d);
    light.blue           = to_imvec4(0x4196df);
    light.magenta        = to_imvec4(0x9870c3);
    light.cyan           = to_imvec4(0x51b891);
    light.white          = to_imvec4(0xc1c1c1);
    light.black_bright   = to_imvec4(0x343434);
    light.red_bright     = to_imvec4(0xee9295);
    light.green_bright   = to_imvec4(0x9fd32f);
    light.yellow_bright  = to_imvec4(0xf0bc7b);
    light.blue_bright    = to_imvec4(0x6daee6);
    light.magenta_bright = to_imvec4(0xb294d2);
    light.cyan_bright    = to_imvec4(0x75c7a8);
    light.white_bright   = to_imvec4(0xdbdbdb);
    themes.emplace("Default (Light)", std::move(light));

    // name: SeaShells
    // https//raw.githubusercontent.com/mbadolato/iTerm2-Color-Schemes/master/schemes/SeaShells.itermcolors
    dark.name           = "Default (Dark)";
    dark.is_dark        = true;
    dark.bg             = to_imvec4(0x061923);
    dark.fg             = to_imvec4(0xe5c49e);
    dark.black          = to_imvec4(0x1d485f);
    dark.red            = to_imvec4(0xdb662d);
    dark.green          = to_imvec4(0x008eab);
    dark.yellow         = to_imvec4(0xfeaf3c);
    dark.blue           = to_imvec4(0x255a62);
    dark.magenta        = to_imvec4(0x77dbf4);
    dark.cyan           = to_imvec4(0x5fb1c2);
    dark.white          = to_imvec4(0xe5c49e);
    dark.black_bright   = to_imvec4(0x545d65);
    dark.red_bright     = to_imvec4(0xdd998a);
    dark.green_bright   = to_imvec4(0x739da8);
    dark.yellow_bright  = to_imvec4(0xfedaae);
    dark.blue_bright    = to_imvec4(0x0bc7e3);
    dark.magenta_bright = to_imvec4(0xc6e8f1);
    dark.cyan_bright    = to_imvec4(0x97b9c0);
    dark.white_bright   = to_imvec4(0xffe9d7);
    themes.emplace("Default (Dark)", std::move(dark));
}

} // namespace lcs::ui
