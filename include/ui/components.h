#pragma once
/*******************************************************************************
 * \file
 * File: include/ui/components.h
 * Created: 06/19/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include "ui/util.h"
#include <imgui.h>

namespace lcs::ui {

void Toast(const char* icon, const char* title, const char* message,
    bool is_error = false);
bool PositionSelector(Point& point, const char* prefix);
State ToggleButton(State, bool clickable = false);
void NodeTypeTitle(Node n);
void NodeTypeTitle(Node n, sockid sock);

template <typename T> void NodeView(NRef<T> base_node, bool has_changes);

template <int SIZE, typename... Args>
bool IconButton(const char* icon, Args... args)
{
    ImGui::BeginGroup();
    bool has_text           = strnlen(get_first<const char*>(args...), 5);
    static char buffer[256] = "##";
    ImVec2 text_s           = { 0, 0 };
    if (has_text) {
        snprintf(buffer + 2, 254, args...);
        text_s = ImGui::CalcTextSize(buffer + 2);
    }
    ImGui::PushFont(get_font(ICON | SIZE));
    ImVec2 icon_s = ImGui::CalcTextSize(icon);
    ImVec2 btn_s  = ImVec2(text_s.x + icon_s.x
             + (has_text ? 1.5f : 1.f) * ImGui::GetStyle().ItemSpacing.x,
         std::max(text_s.y, icon_s.y));
    bool pressed  = ImGui::Button(buffer, btn_s);
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
            ImGui::GetColorU32(ImGuiCol_Text), buffer + 2);
    }
    ImGui::EndGroup();
    return pressed;
}

template <int SIZE, typename... Args>
void IconText(const char* icon, Args... args)
{
    ImGui::BeginGroup();
    bool has_text = strnlen(get_first<const char*>(args...), 5);
    ImGui::PushFont(get_font(ICON | SIZE));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
    if (has_text) {
        ImGui::SameLine();
        ImGui::Text(args...);
    }
    ImGui::EndGroup();
}

template <typename... Args> inline void Section(Args... args)
{
    ImGui::BeginGroup();
    static char buffer[1024] = "##";
    snprintf(buffer + 2, 1022, args...);
    ImGui::PushFont(get_font(FontFlags::LARGE | FontFlags::REGULAR));
    ImGui::TextUnformatted(buffer + 2);
    ImGui::PopFont();
    ImGui::BeginChild(buffer, ImVec2(0, 0),
        ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
}

template <typename... Args> inline void SubSection(Args... args)
{
    ImGui::BeginGroup();
    static char buffer[1024] = "##";
    snprintf(buffer + 2, 1022, args...);
    ImGui::PushFont(get_font(FontFlags::NORMAL | FontFlags::BOLD));
    ImGui::TextUnformatted(buffer + 2);
    ImGui::PopFont();
    ImGui::BeginChild(buffer, ImVec2(0, 0),
        ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
}

inline void EndSection()
{
    ImGui::EndChild();
    ImGui::EndGroup();
}

void ShowIcon(FontFlags size, const char* icon);

template <typename... Args> void Field(Args... args)
{
    ImGui::BeginGroup();
    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
    ImGui::PushStyleColor(ImGuiCol_Text, get_active_style().magenta);
    ImGui::Text(args...);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
}

#define TablePair(KEY, ...)                                                    \
    ImGui::TableNextRow();                                                     \
    ImGui::TableSetColumnIndex(0);                                             \
    KEY;                                                                       \
    ImGui::TableSetColumnIndex(1);                                             \
    __VA_ARGS__
} // namespace lcs::ui
