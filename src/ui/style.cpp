#include "IconsLucide.h"
#include "ui.h"
#include "ui/configuration.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui {
static ImFont* _FONT[FontFlags::FONT_S] = {};

static void _init_fonts(ImGuiIO& io);
static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT]);

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
    style.Alpha = 1.0f;

    style.FrameRounding     = config.rounded_corners;
    style.WindowRounding    = config.rounded_corners;
    style.ChildRounding     = config.rounded_corners;
    style.GrabRounding      = config.rounded_corners;
    style.PopupRounding     = config.rounded_corners;
    style.ScrollbarRounding = config.rounded_corners;
    style.TabRounding       = config.rounded_corners;

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

    if (init) {
        _init_fonts(io);
        io.FontDefault = _FONT[REGULAR | NORMAL];
    }
    io.FontGlobalScale
        = config.scale / 100.f * 2 / 3 /* font downscale factor*/;
    style.ScaleAllSizes(config.scale / 100.f);
}

static void _init_fonts(ImGuiIO& io)
{
    // Load the fonts twice the size and scale them back to have clear visuals.
    float fsize = 16.f * 3 / 2;

    ImFontAtlas* atlas = io.Fonts;
    atlas->Clear();
    io.Fonts->AddFontDefault();
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };

    auto dir         = MISC / "font" / "Archivo" / "ttf";
    auto font_lucide = MISC / "font" / "Lucide" / "Lucide.ttf";

    auto font = dir / "Archivo-Regular.ttf";
#define add_to_atlas(size, ...)                                                \
    atlas->AddFontFromFileTTF(font.c_str(), size, nullptr __VA_ARGS__)
    _FONT[SMALL | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), 0.75 * fsize, nullptr);
    _FONT[NORMAL | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr);
    _FONT[LARGE | REGULAR]
        = atlas->AddFontFromFileTTF(font.c_str(), 1.25 * fsize, nullptr);

    font = dir / "Archivo-Italic.ttf";
    _FONT[SMALL | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), 0.75 * fsize, nullptr);
    _FONT[NORMAL | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr);
    _FONT[LARGE | ITALIC]
        = atlas->AddFontFromFileTTF(font.c_str(), 1.25 * fsize, nullptr);

    font                = dir / "Archivo-Bold.ttf";
    _FONT[SMALL | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[NORMAL | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[LARGE | BOLD] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, atlas->GetGlyphRangesDefault());

    font                         = dir / "Archivo-BoldItalic.ttf";
    _FONT[SMALL | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[NORMAL | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), fsize, nullptr, atlas->GetGlyphRangesDefault());
    _FONT[LARGE | BOLD | ITALIC] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, atlas->GetGlyphRangesDefault());

    font                = MISC / "font" / "Lucide" / "Lucide.ttf";
    _FONT[SMALL | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 0.75 * fsize, nullptr, icon_ranges);
    _FONT[NORMAL | ICON]
        = atlas->AddFontFromFileTTF(font.c_str(), fsize, nullptr, icon_ranges);
    _FONT[LARGE | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 1.25 * fsize, nullptr, icon_ranges);
    _FONT[ULTRA | ICON] = atlas->AddFontFromFileTTF(
        font.c_str(), 2.5 * fsize, nullptr, icon_ranges);
    atlas->Build();
}

void update_fonts() { }

static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT])
{
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

    clr_gui[ImGuiCol_Border]            = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ResizeGrip]        = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ResizeGripHovered] = t.fg;
    clr_gui[ImGuiCol_ResizeGripActive]  = t.fg;
    clr_gui[ImGuiCol_Separator]         = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_SeparatorHovered]  = t.fg;
    clr_gui[ImGuiCol_SeparatorActive]   = t.fg;
    clr_gui[ImGuiCol_TableBorderStrong] = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_TableBorderLight]  = V4MUL(DL(t.black, t.white), 0.9f);

    clr_gui[ImGuiCol_ScrollbarBg] = V4MUL(DL(t.black, t.white), DL(0.5f, 0.8f));
    clr_gui[ImGuiCol_ScrollbarGrab] = V4MUL(DL(t.black, t.white), 0.9f);
    clr_gui[ImGuiCol_ScrollbarGrabHovered]
        = V4MUL(DL(t.black, t.white), DL(0.7f, 1.f));
    clr_gui[ImGuiCol_ScrollbarGrabActive]
        = V4MUL(DL(t.black, t.white), DL(0.75f, 1.05f));

    clr_gui[ImGuiCol_TableHeaderBg]
        = V4MUL(DL(t.yellow, t.yellow_bright), DL(0.8f, 1.f));
    clr_gui[ImGuiCol_TableRowBg]    = t.bg;
    clr_gui[ImGuiCol_TableRowBgAlt] = V4MUL(t.bg, 0.8f);

    clr_gui[ImGuiCol_PlotLines] = V4MUL(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotLinesHovered]
        = V4MUL(DL(t.green, t.yellow_bright), 1.0f);
    clr_gui[ImGuiCol_PlotHistogram] = V4MUL(DL(t.green, t.yellow_bright), 0.9f);
    clr_gui[ImGuiCol_PlotHistogramHovered]
        = V4MUL(DL(t.green, t.yellow_bright), 1.0f);

#define CLRU32(...) ImGui::GetColorU32(__VA_ARGS__)
    clr_node[ImNodesCol_GridBackground]  = CLRU32(V4MUL(t.bg, DL(1.2f, 0.8f)));
    clr_node[ImNodesCol_GridLine]        = CLRU32(V4MUL(t.fg, 0.5f));
    clr_node[ImNodesCol_GridLinePrimary] = CLRU32(ImGuiCol_FrameBgActive);
    clr_node[ImNodesCol_NodeBackground]  = CLRU32(ImGuiCol_WindowBg);
    clr_node[ImNodesCol_NodeBackgroundHovered]  = CLRU32(V4MUL(t.bg, 1.3f));
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
    //  colors[ImGuiCol_DragDropTarget] = theme.cyan_bright;
    //  colors[ImGuiCol_NavHighlight]   = theme.yellow; // renamed in some
    //  versions colors[ImGuiCol_NavWindowingHighlight] = theme.cyan;
    //  colors[ImGuiCol_NavWindowingDimBg]     = V4MUL(theme.bg, 0.5f);
}

} // namespace lcs::ui
