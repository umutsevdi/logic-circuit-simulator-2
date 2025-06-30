#include "IconsLucide.h"
#include "ui.h"
#include "ui/configuration.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>

#define FONTPATH "/usr/share/fonts/UbuntuSans/"
#define _FONT_NAME "UbuntuSansNerdFont-"
#define _BOLD_NAME "Bold"
#define _ITALIC_NAME "Italic"
#define BOLDITALIC BOLD | ITALIC
#define _BOLDITALIC_NAME _BOLD_NAME _ITALIC_NAME
#define _REGULAR_NAME "Regular"
#define LOAD_FONT_FOR(S_FLAG, W_FLAG, ...)                                     \
    if ((_FONT[S_FLAG | W_FLAG] = atlas->AddFontFromFileTTF(                   \
             FONTPATH _FONT_NAME _##W_FLAG##_NAME ".ttf",                      \
             _FONT_SIZES[S_FLAG | W_FLAG], __VA_ARGS__))                       \
        == nullptr) {                                                          \
        L_ERROR("Failed to load the font: " #S_FLAG "|" #W_FLAG);              \
    };

namespace lcs::ui {

static ImFont* _FONT[FontFlags::FONT_S]     = {};
static float _FONT_SIZES[FontFlags::FONT_S] = { 0 };

static void _init_fonts(ImGuiIO& io, Configuration& cfg);
static void _update_fonts(Configuration& cfg);
static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT]);
float get_font_size(int attributes) { return _FONT_SIZES[attributes]; }
ImFont* get_font(int attributes) { return _FONT[attributes]; }

const LcsStyle& get_active_style()
{
    Configuration& config = get_config();
    return ui::get_style(config.preference == Configuration::ALWAYS_LIGHT
            ? config.light_theme
            : config.dark_theme);
}

void set_style(ImGuiIO& io, bool init)
{
    Configuration& config   = get_config();
    config.is_applied       = true;
    ImGuiStyle& style       = ImGui::GetStyle();
    ImNodesStyle& nodestyle = ImNodes::GetStyle();
    _set_colors(style.Colors, nodestyle.Colors);
    //    _set_theme(style, theme, config.preference ==
    //    Configuration::ALWAYS_LIGHT);
    style.Alpha = 1.0f;

    style.FrameRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.WindowRounding    = config.rounded_corners ? 4.0f : 0.f;
    style.ChildRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.GrabRounding      = config.rounded_corners ? 4.0f : 0.f;
    style.PopupRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.ScrollbarRounding = config.rounded_corners ? 4.0f : 0.f;
    style.TabRounding       = config.rounded_corners ? 4.0f : 0.f;

    style.WindowTitleAlign = ImVec2(0.5, 0.5);
    style.WindowPadding    = ImVec2(15, 15);
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

    if (init) {
        _init_fonts(io, config);
    } else {
        _update_fonts(config);
    }

    io.FontDefault = _FONT[REGULAR | NORMAL];
    style.ScaleAllSizes(config.scale / 100.f);
}

static void _init_fonts(ImGuiIO& io, Configuration& cfg)
{
    _FONT_SIZES[SMALL | REGULAR]     = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | BOLD]        = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | ITALIC]      = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | BOLDITALIC]  = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | ICON]        = cfg.font_size - 4.0f;
    _FONT_SIZES[NORMAL | REGULAR]    = cfg.font_size;
    _FONT_SIZES[NORMAL | BOLD]       = cfg.font_size;
    _FONT_SIZES[NORMAL | ITALIC]     = cfg.font_size;
    _FONT_SIZES[NORMAL | BOLDITALIC] = cfg.font_size;
    _FONT_SIZES[NORMAL | ICON]       = cfg.font_size;
    _FONT_SIZES[LARGE | REGULAR]     = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | BOLD]        = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | ITALIC]      = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | BOLDITALIC]  = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | ICON]        = cfg.font_size + 8.f;
    _FONT_SIZES[ULTRA | ICON]        = 40.f;

    ImFontAtlas* atlas = io.Fonts;
    atlas->Clear();
    io.Fonts->AddFontDefault();
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
#define FONTPATH "/usr/share/fonts/UbuntuSans/"

    LOAD_FONT_FOR(SMALL, REGULAR, nullptr);
    LOAD_FONT_FOR(SMALL, BOLD, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(SMALL, ITALIC, nullptr);
    LOAD_FONT_FOR(SMALL, BOLDITALIC, nullptr, atlas->GetGlyphRangesDefault());

    LOAD_FONT_FOR(NORMAL, REGULAR, nullptr);
    LOAD_FONT_FOR(NORMAL, BOLD, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(NORMAL, ITALIC, nullptr);
    LOAD_FONT_FOR(NORMAL, BOLDITALIC, nullptr, atlas->GetGlyphRangesDefault());

    LOAD_FONT_FOR(LARGE, REGULAR, nullptr);
    LOAD_FONT_FOR(LARGE, BOLD, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(LARGE, ITALIC, nullptr);
    LOAD_FONT_FOR(LARGE, BOLDITALIC, nullptr, atlas->GetGlyphRangesDefault());
#undef FONTPATH
#define FONTPATH "../misc/" FONT_ICON_FILE_NAME_LC

    _FONT[SMALL | ICON] = io.Fonts->AddFontFromFileTTF(
        FONTPATH, _FONT_SIZES[ICON | SMALL], nullptr, icon_ranges);
    _FONT[NORMAL | ICON] = io.Fonts->AddFontFromFileTTF(
        FONTPATH, _FONT_SIZES[ICON | NORMAL], nullptr, icon_ranges);
    _FONT[LARGE | ICON] = io.Fonts->AddFontFromFileTTF(
        FONTPATH, _FONT_SIZES[ICON | LARGE], nullptr, icon_ranges);
    _FONT[ULTRA | ICON] = io.Fonts->AddFontFromFileTTF(
        FONTPATH, _FONT_SIZES[ICON | ULTRA], nullptr, icon_ranges);
    atlas->Build();
}

static void _update_fonts(Configuration& cfg)
{
    _FONT_SIZES[SMALL | REGULAR]     = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | BOLD]        = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | ITALIC]      = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | BOLDITALIC]  = cfg.font_size - 4.0f;
    _FONT_SIZES[SMALL | ICON]        = cfg.font_size - 4.0f;
    _FONT_SIZES[NORMAL | REGULAR]    = cfg.font_size;
    _FONT_SIZES[NORMAL | BOLD]       = cfg.font_size;
    _FONT_SIZES[NORMAL | ITALIC]     = cfg.font_size;
    _FONT_SIZES[NORMAL | BOLDITALIC] = cfg.font_size;
    _FONT_SIZES[NORMAL | ICON]       = cfg.font_size;
    _FONT_SIZES[LARGE | REGULAR]     = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | BOLD]        = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | ITALIC]      = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | BOLDITALIC]  = cfg.font_size + 8.f;
    _FONT_SIZES[LARGE | ICON]        = cfg.font_size + 8.f;
    _FONT_SIZES[ULTRA | ICON]        = 40.f;

    _FONT[SMALL | REGULAR]->FontSize     = cfg.font_size - 4.0f;
    _FONT[SMALL | BOLD]->FontSize        = cfg.font_size - 4.0f;
    _FONT[SMALL | ITALIC]->FontSize      = cfg.font_size - 4.0f;
    _FONT[SMALL | BOLDITALIC]->FontSize  = cfg.font_size - 4.0f;
    _FONT[SMALL | ICON]->FontSize        = cfg.font_size - 4.0f;
    _FONT[NORMAL | REGULAR]->FontSize    = cfg.font_size;
    _FONT[NORMAL | BOLD]->FontSize       = cfg.font_size;
    _FONT[NORMAL | ITALIC]->FontSize     = cfg.font_size;
    _FONT[NORMAL | BOLDITALIC]->FontSize = cfg.font_size;
    _FONT[NORMAL | ICON]->FontSize       = cfg.font_size;
    _FONT[LARGE | REGULAR]->FontSize     = cfg.font_size + 8.f;
    _FONT[LARGE | BOLD]->FontSize        = cfg.font_size + 8.f;
    _FONT[LARGE | ITALIC]->FontSize      = cfg.font_size + 8.f;
    _FONT[LARGE | BOLDITALIC]->FontSize  = cfg.font_size + 8.f;
    _FONT[LARGE | ICON]->FontSize        = cfg.font_size + 8.f;
}

static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT])
{
#define V4MUL(vec, pct, ...)                                                   \
    ImVec4((vec).x*(pct), (vec).y*(pct), (vec).z*(pct), (vec).w __VA_ARGS__)
#define DL(_a, _b) (t.is_dark ? (_a) : (_b))
    const LcsStyle& t = get_active_style();

    clr_gui[ImGuiCol_WindowBg]       = t.bg;
    clr_gui[ImGuiCol_Text]           = t.fg;
    clr_gui[ImGuiCol_TextDisabled]   = V4MUL(t.fg, 0.7f);
    clr_gui[ImGuiCol_TextSelectedBg] = V4MUL(t.blue, 0.8f);
    clr_gui[ImGuiCol_TextLink]       = t.blue_bright;

    // Headers
    clr_gui[ImGuiCol_MenuBarBg] = V4MUL(t.bg, DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_Header]    = V4MUL(DL(t.black, t.white), DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_HeaderHovered]
        = V4MUL(DL(t.black, t.white), DL(0.7f, 1.f));
    clr_gui[ImGuiCol_HeaderActive]
        = V4MUL(DL(t.black, t.white), DL(0.75f, 1.05f));

    // Buttons
    clr_gui[ImGuiCol_Button]
        = V4MUL(DL(t.yellow, t.yellow_bright), DL(0.5f, 0.9f));
    clr_gui[ImGuiCol_ButtonHovered]
        = V4MUL(DL(t.yellow, t.yellow_bright), DL(0.8f, 1.0f));
    clr_gui[ImGuiCol_ButtonActive]
        = V4MUL(DL(t.yellow, t.yellow_bright), DL(0.8f, 1.1f));
    clr_gui[ImGuiCol_CheckMark] = t.green;

    clr_gui[ImGuiCol_DockingPreview] = V4MUL(t.blue_bright, 1.f, *0.5f);
    clr_gui[ImGuiCol_DockingEmptyBg] = V4MUL(t.blue_bright, 1.f, *0.5f);

    // Frame BG
    clr_gui[ImGuiCol_FrameBg]        = V4MUL(t.bg, DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_FrameBgHovered] = V4MUL(t.bg, DL(1.7f, 1.2f));
    clr_gui[ImGuiCol_FrameBgActive]  = V4MUL(t.bg, DL(1.7f, 1.2f));
    clr_gui[ImGuiCol_ChildBg]        = V4MUL(t.bg, DL(0.75f, 0.9f));
    clr_gui[ImGuiCol_PopupBg]        = V4MUL(t.bg, DL(0.75f, 0.9f));
    clr_gui[ImGuiCol_ModalWindowDimBg]
        = V4MUL(DL(t.black, t.white), 0.5f, *0.5f);

    // Tabs
    clr_gui[ImGuiCol_Tab]                = V4MUL(DL(t.black, t.white), 0.8f);
    clr_gui[ImGuiCol_TabHovered]         = V4MUL(DL(t.black, t.white), 1.f);
    clr_gui[ImGuiCol_TabActive]          = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TabUnfocused]       = V4MUL(DL(t.black, t.white), 0.5f);
    clr_gui[ImGuiCol_TabUnfocusedActive] = V4MUL(DL(t.black, t.white), 0.7f);

    // Title
    clr_gui[ImGuiCol_TitleBg]          = V4MUL(DL(t.black, t.white), 0.8f);
    clr_gui[ImGuiCol_TitleBgActive]    = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TitleBgCollapsed] = t.bg;

    clr_gui[ImGuiCol_Border]            = V4MUL(t.fg, 1.2f);
    clr_gui[ImGuiCol_ResizeGrip]        = V4MUL(t.fg, 1.2f);
    clr_gui[ImGuiCol_ResizeGripHovered] = t.fg;
    clr_gui[ImGuiCol_ResizeGripActive]  = t.fg;
    clr_gui[ImGuiCol_Separator]         = V4MUL(t.fg, 1.2f);
    clr_gui[ImGuiCol_SeparatorHovered]  = t.fg;
    clr_gui[ImGuiCol_SeparatorActive]   = t.fg;

    clr_gui[ImGuiCol_TableHeaderBg]
        = V4MUL(DL(t.yellow, t.yellow_bright), DL(0.8f, 1.f));
    clr_gui[ImGuiCol_TableBorderStrong] = t.fg;
    clr_gui[ImGuiCol_TableBorderLight]  = t.fg;
    clr_gui[ImGuiCol_TableRowBg]        = V4MUL(t.bg, 0.7f);
    clr_gui[ImGuiCol_TableRowBgAlt]     = V4MUL(t.bg, 0.5f);

    clr_gui[ImGuiCol_PlotLines] = V4MUL(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotLinesHovered]
        = V4MUL(DL(t.green, t.yellow_bright), 1.0f);
    clr_gui[ImGuiCol_PlotHistogram] = V4MUL(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotHistogramHovered]
        = V4MUL(DL(t.green, t.yellow_bright), 1.0f);

#define CLRU32(...) ImGui::GetColorU32(__VA_ARGS__)
    clr_node[ImNodesCol_GridBackground]
        = CLRU32(V4MUL(t.bg, DL(1.2f, 0.8f)));
    clr_node[ImNodesCol_GridLine]              = CLRU32(V4MUL(t.fg, 0.5f));
    clr_node[ImNodesCol_GridLinePrimary]       = CLRU32(ImGuiCol_FrameBgActive);
    clr_node[ImNodesCol_NodeBackground]        = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_NodeBackgroundHovered] = CLRU32(V4MUL(t.bg, 1.3f));
    clr_node[ImNodesCol_NodeBackgroundSelected] = CLRU32(V4MUL(t.bg, 1.5f));

    clr_node[ImNodesCol_NodeOutline] = CLRU32(V4MUL(t.fg, 0.5f));

    clr_node[ImNodesCol_TitleBar]         = CLRU32(ImGuiCol_Tab);
    clr_node[ImNodesCol_TitleBarHovered]  = CLRU32(ImGuiCol_TabHovered);
    clr_node[ImNodesCol_TitleBarSelected] = CLRU32(ImGuiCol_TabActive);
    clr_node[ImNodesCol_Link]             = CLRU32(ImGuiCol_Button);
    clr_node[ImNodesCol_LinkHovered]      = CLRU32(ImGuiCol_ButtonHovered);
    clr_node[ImNodesCol_LinkSelected]     = CLRU32(ImGuiCol_ButtonActive);
    clr_node[ImNodesCol_Pin]              = CLRU32(V4MUL(t.yellow, 1.0f));
    clr_node[ImNodesCol_PinHovered]  = CLRU32(V4MUL(t.yellow_bright, 1.2f));
    clr_node[ImNodesCol_BoxSelector] = CLRU32(ImGuiCol_DockingPreview);
    clr_node[ImNodesCol_BoxSelectorOutline] = CLRU32(ImGuiCol_DockingEmptyBg);

    clr_node[ImNodesCol_MiniMapBackground]        = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapBackgroundHovered] = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapOutline]           = CLRU32(ImGuiCol_FrameBg);
    clr_node[ImNodesCol_MiniMapOutlineHovered]
        = CLRU32(ImGuiCol_FrameBgHovered);

    clr_node[ImNodesCol_MiniMapNodeBackground] = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_MiniMapNodeBackgroundHovered]
        = CLRU32(V4MUL(DL(t.black, t.white), 1.1f));
    clr_node[ImNodesCol_MiniMapNodeBackgroundSelected]
        = CLRU32(V4MUL(DL(t.black, t.white), 1.1f));
    //    clr_node[ImNodesCol_MiniMapNodeOutline] =
    //    clr_node[ImNodesCol_MiniMapLink] =
    //    clr_node[ImNodesCol_MiniMapLinkSelected] =
//  clr_node[ImNodesCol_MiniMapCanvas]        = CLRU32(ImGuiCol_FrameBg);
//  clr_node[ImNodesCol_MiniMapCanvasOutline] = CLRU32(ImGuiCol_Text);
#undef CLRU32
//  // Scrollbars
//  colors[ImGuiCol_ScrollbarBg]          = V4MUL(theme.bg,
//  DL(0.5f, 1.5f)); colors[ImGuiCol_ScrollbarGrab]        = theme.blue;
//  colors[ImGuiCol_ScrollbarGrabHovered] = theme.blue_bright;
//  colors[ImGuiCol_ScrollbarGrabActive]  = theme.blue_bright;

//  // Widgets
//  colors[ImGuiCol_SliderGrab]       = theme.green;
//  0j
//  colors[ImGuiCol_SliderGrabActive] = theme.green_bright;

//  // Separators & resize grips

//  // Plots
//  colors[ImGuiCol_PlotLines]            = theme.fg;
//  colors[ImGuiCol_PlotLinesHovered]     = is_dark ? theme.white :
//  theme.black; colors[ImGuiCol_PlotHistogram]        = theme.yellow;
//  colors[ImGuiCol_PlotHistogramHovered] = theme.yellow_bright;

//  // Tables
//
//
//
//
//

//  // Links & selection

//  // Drag & drop, navigation, modals
//  colors[ImGuiCol_DragDropTarget] = theme.cyan_bright;
//  colors[ImGuiCol_NavHighlight]   = theme.yellow; // renamed in some
//  versions colors[ImGuiCol_NavWindowingHighlight] = theme.cyan;
//  colors[ImGuiCol_NavWindowingDimBg]     = V4MUL(theme.bg, 0.5f);
#undef V4MUL
#undef DL
}
} // namespace lcs::ui
