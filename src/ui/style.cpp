#include "imnodes.h"
#include "ui.h"

#include "common.h"
#include "ui/util.h"
#include <imgui.h>
#define FONTPATH "/usr/share/fonts/UbuntuMono/"
#define _FONT_NAME "UbuntuMonoNerdFont-"
#define _BOLD_NAME "Bold"
#define _ITALIC_NAME "Italic"
#define BOLDITALIC BOLD | ITALIC
#define _BOLDITALIC_NAME _BOLD_NAME _ITALIC_NAME
#define _REGULAR_NAME "Regular"
#define LOAD_FONT_FOR(S_FLAG, W_FLAG, ...)                                     \
    if ((_FONT[S_FLAG | W_FLAG] = atlas->AddFontFromFileTTF(                   \
             FONTPATH _FONT_NAME _##W_FLAG##_NAME ".ttf", __VA_ARGS__))        \
        == nullptr) {                                                          \
        L_ERROR("Failed to load the font: " #S_FLAG "|" #W_FLAG);              \
    };

namespace lcs::ui {

static ImFont* _FONT[font_flags_t::FONT_S] = { 0 };

static void _init_fonts(ImGuiIO& io)
{
    ImFontAtlas* atlas = io.Fonts;
    atlas->Clear();

    LOAD_FONT_FOR(SMALL, REGULAR, 12.f);
    LOAD_FONT_FOR(SMALL, BOLD, 12.f, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(SMALL, ITALIC, 12.f);
    LOAD_FONT_FOR(
        SMALL, BOLDITALIC, 10.f, nullptr, atlas->GetGlyphRangesDefault());

    LOAD_FONT_FOR(NORMAL, REGULAR, 15.f);
    LOAD_FONT_FOR(NORMAL, BOLD, 15.f, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(NORMAL, ITALIC, 15.f);
    LOAD_FONT_FOR(
        SMALL, BOLDITALIC, 16.f, nullptr, atlas->GetGlyphRangesDefault());

    LOAD_FONT_FOR(LARGE, REGULAR, 24.f);
    LOAD_FONT_FOR(LARGE, BOLD, 24.f, nullptr, atlas->GetGlyphRangesDefault());
    LOAD_FONT_FOR(LARGE, ITALIC, 24.f);
    LOAD_FONT_FOR(
        SMALL, BOLDITALIC, 24.f, nullptr, atlas->GetGlyphRangesDefault());
}

ImFont* get_font(int attributes) { return _FONT[attributes]; }

void set_style(ImGuiIO& io, int alpha, bool& is_dark)
{
    ImGuiStyle& style = ImGui::GetStyle();

    // light style from Pacôme Danhiez (user itamago)
    // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha                         = 1.0f;
    style.FrameRounding                 = 3.0f;
    style.Colors[ImGuiCol_Text]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]     = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    //   style.Colors[ImGuiCol_ChildWindowBg]  = ImVec4(0.00f, 0.00f, 0.00f,
    //   0.00f);
    style.Colors[ImGuiCol_PopupBg]        = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border]         = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow]   = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]  = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]        = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]
        = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]     = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]   = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]
        = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]
        = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    //    style.Colors[ImGuiCol_ComboBg]    = ImVec4(0.86f, 0.86f, 0.86f,
    //    0.99f);
    style.Colors[ImGuiCol_CheckMark]  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]
        = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]  = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header]        = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive]  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    //    style.Colors[ImGuiCol_Column]        = ImVec4(0.39f, 0.39f,
    //    0.39f, 1.00f);
    //  style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f,
    //  0.78f); style.Colors[ImGuiCol_ColumnActive]  = ImVec4(0.26f, 0.59f,
    //  0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered]
        = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]
        = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    //  style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
    //  style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f,
    //  0.36f, 1.00f);
    //    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f,
    //    0.36f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]
        = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]
        = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    //    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f,
    //    0.20f, 0.35f);

    if (is_dark) {
        for (int i = 0; i <= ImGuiCol_COUNT; i++) {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

            if (S < 0.1f) {
                V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
            if (col.w < 1.00f) {
                col.w *= alpha;
            }
        }
    } else {
        for (int i = 0; i <= ImGuiCol_COUNT; i++) {
            ImVec4& col = style.Colors[i];
            if (col.w < 1.00f) {
                col.x *= alpha;
                col.y *= alpha;
                col.z *= alpha;
                col.w *= alpha;
            }
        }
    }
    _init_fonts(io);
    ImNodes::PushColorStyle(
        ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
    ImNodes::PushColorStyle(
        ImNodesCol_LinkHovered, IM_COL32(200, 200, 200, 255));
}
} // namespace lcs::ui
