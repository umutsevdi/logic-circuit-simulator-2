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

float get_font_size(int attributes) { return _FONT_SIZES[attributes]; }
ImFont* get_font(int attributes) { return _FONT[attributes]; }

void set_style(ImGuiIO& io, bool init)
{
    Configuration& config      = get_config();
    config.is_applied          = true;
    ImGuiStyle& style          = ImGui::GetStyle();
    const LcsStyle& stylesheet = ui::get_style(
        config.preference == Configuration::ALWAYS_LIGHT ? config.light_theme
                                                         : config.dark_theme);
    style.Alpha = 1.0f;

    style.FrameRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.WindowRounding    = config.rounded_corners ? 4.0f : 0.f;
    style.ChildRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.GrabRounding      = config.rounded_corners ? 4.0f : 0.f;
    style.PopupRounding     = config.rounded_corners ? 4.0f : 0.f;
    style.ScrollbarRounding = config.rounded_corners ? 4.0f : 0.f;
    style.TabRounding       = config.rounded_corners ? 4.0f : 0.f;

    style.WindowPadding    = ImVec2(15, 15);
    style.FramePadding     = ImVec2(5, 5);
    style.ItemSpacing      = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 2);
    style.IndentSpacing    = 25.0f;
    style.ScrollbarSize    = 10.0f;
    style.GrabMinSize      = 5.0f;

    if (init) {
        _init_fonts(io, config);
    } else {
        _update_fonts(config);
    }

    io.FontDefault = _FONT[REGULAR | NORMAL];
    ImNodes::PushColorStyle(
        ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
    ImNodes::PushColorStyle(
        ImNodesCol_LinkHovered, IM_COL32(200, 200, 200, 255));
}
} // namespace lcs::ui
