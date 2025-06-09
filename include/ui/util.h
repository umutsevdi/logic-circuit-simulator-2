#pragma once
/*******************************************************************************
 * \file
 * File: /home/umutsevdi/src/dev/lc-simulator-2/include/ui/util.h
 * Created: 06/06/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include <imgui.h>
namespace lcs::ui {

enum font_flags_t {
    /** Size Flag: SMALL */
    SMALL = 0b00000,
    /** Size Flag: NORMAL */
    NORMAL = 0b00001,
    /** Size Flag: LARGE */
    LARGE = 0b00010,
    /** Format Flag: REGULAR */
    REGULAR = 0b00000,
    /** Format Flag: ITALIC */
    ITALIC = 0b001000,
    /** Format Flag: BOLD */
    BOLD = 0b010000,
    /** Format Flag: ICON */
    ICON = 0b100000,

    // Small/Regular/Large * BOLD|ITALIC|REGULAR|BOLD-ITALIC
    FONT_S = ICON | LARGE | 1
};

int encode_pair(Node node, sockid sock, bool is_out);
Node decode_pair(int pair_code, sockid* sock = nullptr, bool* is_out = nullptr);

ImFont* get_font(int attributes);
float get_font_size(int attributes);
constexpr ImVec4 NodeType_to_color(mode_t type)
{
    switch (type) {
    case NodeType::GATE: return ImVec4(0, 1, 1, 1);
    case NodeType::INPUT: return ImVec4(0.3, 0.3, 0.7, 1);
    case NodeType::OUTPUT: return ImVec4(0.6, 0.0, 0.6, 1);
    case NodeType::COMPONENT: return ImVec4(0.3, 0.7, 0.3, 1);
    default: return ImVec4(0.8, 0.6, 0.1, 1);
    }
}

void PositionSelector(NRef<BaseNode> node, const char* prefix);
State ToggleButton(State, bool clickable = false);
void NodeTypeTitle(Node n);
void NodeTypeTitle(Node n, sockid sock);

#define ShowIcon(SIZE, NAME)                                                   \
    ImGui::PushFont(get_font(font_flags_t::ICON | SIZE));                      \
    ImGui::Text("%s", NAME);                                                   \
    ImGui::PopFont();
#define Field(...)                                                             \
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));      \
    ImGui::TextColored(ImVec4(200, 200, 0, 255), __VA_ARGS__);                 \
    ImGui::PopFont()
#define TablePair(KEY, ...)                                                    \
    ImGui::TableNextRow();                                                     \
    ImGui::TableSetColumnIndex(0);                                             \
    KEY;                                                                       \
    ImGui::TableSetColumnIndex(1);                                             \
    __VA_ARGS__

template <int SIZE, typename... Args>
bool IconButton(const char* label, const char* icon, Args... args)
{

    bool has_text = strnlen(get_first<const char*>(args...), 5);
    static char buffer[256];
    ImVec2 text_s = { 0, 0 };
    if (has_text) {
        snprintf(buffer, 256, args...);
        text_s = ImGui::CalcTextSize(buffer);
    }
    ImGui::PushFont(get_font(ICON | SIZE));
    ImVec2 icon_s = ImGui::CalcTextSize(icon);
    ImVec2 btn_s  = ImVec2(text_s.x + icon_s.x
             + (has_text ? 1.5f : 1.f) * ImGui::GetStyle().ItemSpacing.x,
         std::max(text_s.y, icon_s.y));
    bool pressed  = ImGui::Button(label, btn_s);
    ImVec2 btnpos = ImGui::GetItemRectMin();

    ImGui::GetWindowDrawList()->AddText(
        ImVec2(btnpos.x + ImGui::GetStyle().ItemSpacing.x / 2,
            btnpos.y + (btn_s.y - icon_s.y) / 2),
        ImGui::GetColorU32(ImGuiCol_Text), icon);
    ImGui::PopFont();
    if (has_text) {
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(btnpos.x + icon_s.x + ImGui::GetStyle().ItemSpacing.x,
                btnpos.y + (btn_s.y - text_s.y) / 2),
            ImGui::GetColorU32(ImGuiCol_Text), buffer);
    }
    return pressed;
}

template <int SIZE, typename... Args>
void IconText(const char* icon, Args... args)
{
    bool has_text = strnlen(get_first<const char*>(args...), 5);
    ImGui::PushFont(get_font(ICON | SIZE));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
    if (has_text) {
        ImGui::SameLine();
        ImGui::Text(args...);
    }
}

void SceneType(NRef<Scene>);

} // namespace lcs::ui
