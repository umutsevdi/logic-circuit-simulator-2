#include "ui/util.h"
#include <imgui.h>
namespace lcs::ui {

void ToggleButton(State state)
{
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    switch (state) {
    case State::TRUE: ImGui::TextColored(ImVec4(0, 1, 0, 1), "TRUE"); break;
    case State::FALSE: ImGui::TextColored(ImVec4(1, 0, 0, 1), "FALSE"); break;
    case State::DISABLED:
        ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISABLED");
        break;
    default: break;
    }
    ImGui::PopFont();
}

void ToggleButton(NRef<InputNode> node)
{
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    switch (node->get()) {
    case State::TRUE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("TRUE ")) {
            node->toggle();
            io::scene::notify_change();
        };
        ImGui::PopStyleColor();
        break;
    case State::FALSE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("FALSE")) {
            node->toggle();
            io::scene::notify_change();
        };
        break;
    case State::DISABLED:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.3, 0.3, 1));
        ImGui::Button("DISABLED");
        break;
    default: break;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
}

void NodeTypeTitle(Node n)
{
    ImGui::TextColored(
        NodeType_to_color(n.type), "%s", NodeType_to_str(n.type));
    ImGui::SameLine();
    ImGui::Text("@");
    ImGui::SameLine();
    ImGui::TextColored(
        ImVec4(0, 1.0, 0, 1), "%s", std::to_string(n.id).c_str());
}

void NodeTypeTitle(Node n, sockid sock)
{
    ImGui::PushFont(get_font(font_flags_t::REGULAR | font_flags_t::NORMAL));
    NodeTypeTitle(n);
    ImGui::SameLine();
    ImGui::Text("sock:");
    ImGui::SameLine();
    ImGui::TextColored(
        ImVec4(0.5f, 0.5f, 1.0f, 1.00f), "%s", std::to_string(sock).c_str());
    ImGui::PopFont();
}

} // namespace lcs::ui
