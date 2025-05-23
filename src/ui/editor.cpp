#include "ui.h"

#include "common.h"
#include <imgui.h>

bool static show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

namespace lcs::ui {

static ImFont* _FONT[font_t::FONT_S] = { 0 };

bool loop(ImGuiIO& io)
{
    static float f     = 0.0f;
    static int counter = 0;

    ImGui::Begin("Party!"); // Create a window called "Hello,
                            // world!" and append into it.

    ImGui::PushFont(_FONT[SMALL]);
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(_FONT[LARGE]);
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(_FONT[REGULAR]);
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(_FONT[W2]);
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();

    ImGui::Checkbox("Demo Window",
        &show_demo_window); // Edit bools storing our window
                            // open/close state

    ImGui::SliderFloat("float", &f, 0.0f,
        1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color",
        (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                 // widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    if ((ImGui::Checkbox("DarkModeButton", &is_dark))) {
        ImGui::SameLine();
        ImGui::Text("Dark Mode: %s", is_dark ? "true" : "false");
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
    return show_demo_window;
}

#define FONTPATH "/usr/share/fonts/UbuntuMono/"
static void _init_fonts(ImGuiIO& io)
{

    ImFontAtlas* fontAtlas = io.Fonts;
    fontAtlas->Clear();
    if ((_FONT[font_t::SMALL] = fontAtlas->AddFontFromFileTTF(
             FONTPATH "UbuntuMonoNerdFont-Regular.ttf", 10.0f))
        == nullptr) {
        L_ERROR("Failed to load the font: SMALL");
    };
    if ((_FONT[font_t::REGULAR] = fontAtlas->AddFontFromFileTTF(
             FONTPATH "UbuntuMonoNerdFont-Regular.ttf", 12.0f))
        == nullptr) {
        L_ERROR("Failed to load the font: REGULAR");
    };
    if ((_FONT[font_t::LARGE] = fontAtlas->AddFontFromFileTTF(
             FONTPATH "UbuntuMonoNerdFont-Regular.ttf", 16.0f))
        == nullptr) {
        L_ERROR("Failed to load the font: LARGE");
    };
    if ((_FONT[font_t::W2] = fontAtlas->AddFontFromFileTTF(FONTPATH
             "UbuntuMonoNerdFont-Bold.ttf",
             16.0f, nullptr, fontAtlas->GetGlyphRangesDefault()))
        == nullptr) {
        L_ERROR("Failed to load the font: BOLD");
    };
}
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

            if (S < 0.1f) { V = 1.0f - V; }
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
            if (col.w < 1.00f) { col.w *= alpha; }
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
}
} // namespace lcs::ui
