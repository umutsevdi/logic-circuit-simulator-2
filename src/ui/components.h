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

#include <imgui.h>
#include "configuration.h"
#include "core.h"

namespace lcs::ui {

enum FontFlags {
    /** Size Flag: SMALL */
    SMALL = 0b00000,
    /** Size Flag: NORMAL */
    NORMAL = 0b00001,
    /** Size Flag: LARGE */
    LARGE = 0b00010,
    /** Size Flag: ULTRA: Only for icons */
    ULTRA = 0b00100,
    /** Format Flag: REGULAR */
    REGULAR = 0b00000,
    /** Format Flag: ITALIC */
    ITALIC = 0b001000,
    /** Format Flag: BOLD */
    BOLD = 0b010000,
    /** Format Flag: ICON */
    ICON = 0b100000,

    // Small/Regular/Large * BOLD|ITALIC|REGULAR|BOLD-ITALIC
    FONT_S = ICON | ULTRA | 1
};

ImFont* get_font(int attributes);
float get_font_size(int attributes);

void SceneType(NRef<Scene>);

int hash_pair(Node node, sockid sock, bool is_out);

struct ImageHandle {
    uint32_t gl_id = 0;
    int w          = 0;
    int h          = 0;
    inline ImVec2 size(void) const { return ImVec2 { (float)w, (float)h }; }
};

/**
 * Read a texture from given given vector and register as an OpenGL texture.
 * @param key to obtain image back
 * @param buffer to read from
 *
 */
bool load_texture(const std::string& key, std::vector<unsigned char>& buffer);

/**
 * Read a texture from given file and register as an OpenGL texture.
 * @param key to obtain image back
 * @param file_path to read from
 *
 */
bool load_texture(const std::string& key, const std::string& file_path);

/**
 * Get a reference to a registered image by given key.
 * @param key lookup key
 * @returns ImageHandle
 */
const ImageHandle* get_texture(const std::string& key);

void Toast(const char* icon, const char* title, const char* message,
    bool is_error = false);
bool PositionSelector(Point& point, const char* prefix);
State ToggleButton(State, bool clickable = false);
void NodeTypeTitle(Node n);
void NodeTypeTitle(Node n, sockid sock);

inline void ShowIcon(FontFlags size, const char* icon)
{
    ImGui::PushFont(get_font(FontFlags::ICON | size));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
}

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

template <typename... Args> inline void Field(Args... args)
{
    ImGui::BeginGroup();
    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
    ImGui::PushStyleColor(ImGuiCol_Text, get_active_style().cyan);
    ImGui::Text(args...);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
}

#define AnonTable(ID, _KEYWIDTH, ...)                                          \
    if (ImGui::BeginTable("##" ID, 2, ImGuiTableFlags_BordersInnerV)) {        \
        ImGui::TableSetupColumn(                                               \
            "##Key", ImGuiTableColumnFlags_WidthFixed, _KEYWIDTH);             \
        ImGui::NextColumn();                                                   \
        ImGui::TableSetupColumn(                                               \
            "##Value", ImGuiTableColumnFlags_WidthStretch);                    \
        __VA_ARGS__                                                            \
        ImGui::EndTable();                                                     \
    }

#define TableKey(KEY)                                                          \
    ImGui::TableNextRow();                                                     \
    ImGui::TableSetColumnIndex(0);                                             \
    KEY;                                                                       \
    ImGui::TableSetColumnIndex(1);

#define TablePair(KEY, VALUE)                                                  \
    TableKey(KEY);                                                             \
    VALUE

template <typename... Args>
inline bool BeginTooltip(const char* icon, Args... args)
{
    bool bgn = ImGui::BeginTooltip();
    if (bgn) {
        ImGui::PushFont(get_font(NORMAL | BOLD));
        if (icon != nullptr) {
            ShowIcon(FontFlags::LARGE, icon);
        }
        ImGui::SameLine();
        ImGui::Text(args...);
        ImGui::PopFont();
        ImGui::Separator();
    }
    return bgn;
}

inline void EndTooltip(const char* shortcut = nullptr)
{
    ImGui::Spacing();
    if (shortcut != nullptr) {
        ImGui::TextColored(
            get_active_style().yellow, _("Shortcut: %s"), shortcut);
    }
    ImGui::EndTooltip();
}

#define HINT(SHORTCUT, TITLE, ...)                                             \
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_None)) {                        \
        BeginTooltip(nullptr, TITLE);                                          \
        ImGui::Text(__VA_ARGS__);                                              \
        EndTooltip(SHORTCUT);                                                  \
    }

} // namespace lcs::ui
