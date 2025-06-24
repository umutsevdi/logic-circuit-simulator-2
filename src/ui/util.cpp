#include "ui/util.h"
#include "core.h"
#include "io.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui {

void PositionSelector(NRef<BaseNode> node, const char* prefix)
{
    const static ImVec2 __selector_size = ImGui::CalcTextSize("-000000000000");

    std::string s_prefix = "##";
    s_prefix += prefix;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("( x: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(__selector_size.x);
    if (ImGui::InputInt(
            (s_prefix + "X").c_str(), &node->point.x, 1.0f, 10.0f, 0)) {
        io::scene::notify_change();
    };
    ImGui::SameLine();
    ImGui::Text(", y:");
    ImGui::SameLine();
    if (ImGui::InputInt(
            (s_prefix + "Y").c_str(), &node->point.y, 1.0f, 10.0f, 0)) {
        io::scene::notify_change();
    };
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text(")");
}

State ToggleButton(State state, bool clickable)
{
    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::SMALL));
    switch (state) {
    case State::TRUE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            if (ImGui::Button("TRUE ")) {
                state = FALSE;
            };
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "TRUE");
        }
        break;
    case State::FALSE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
            if (ImGui::Button("FALSE")) {
                state = TRUE;
            };
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "FALSE");
        }
        break;
    case State::DISABLED:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.3, 0.3, 1));
            ImGui::Button("DISCONNECTED");
        } else {
            ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISCONNECTED");
        }
        break;
    }
    if (clickable) {
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    return state;
}

void ToggleButton(NRef<InputNode> node)
{
    State s_old = node->get();

    if (s_old != ToggleButton(node->get(), true)) {
        node->toggle();
        io::scene::notify_change();
    }
}

void NodeTypeTitle(Node n)
{
    static char buffer[256];
    ImGui::PushStyleColor(ImGuiCol_TextLink, NodeType_to_color(n.type));
    snprintf(buffer, 256, "%s@%u", NodeType_to_str_full(n.type), n.id);
    if (ImGui::TextLink(buffer)) {
        ImNodes::ClearNodeSelection();
        switch (n.type) {
        case COMPONENT_INPUT:
        case COMPONENT_OUTPUT:
            ImNodes::SelectNode(Node { 0, n.type }.numeric());
            break;
        default: ImNodes::SelectNode(n.numeric()); break;
        }
    };
    ImGui::PopStyleColor();
}

void NodeTypeTitle(Node n, sockid sock)
{
    ImGui::PushFont(get_font(FontFlags::REGULAR | FontFlags::NORMAL));
    NodeTypeTitle(n);
    ImGui::SameLine();
    ImGui::Text("sock:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.00f), "%u", sock);
    ImGui::PopFont();
}

int encode_pair(Node node, sockid sock, bool is_out)
{
    lcs_assert(node.id < 0xFFFF);
    int x = node.id | (node.type << 16) | (sock << 20);
    if (is_out) {
        x |= 1 << 31;
    }
    return x;
}

Node decode_pair(int code, sockid* sock, bool* is_out)
{
    if (is_out != nullptr) {
        *is_out = code >> 31;
    }
    if (sock != nullptr) {
        *sock = (code >> 20) & 0xFF;
    }
    return Node { static_cast<uint16_t>(code & 0xFFFF),
        static_cast<NodeType>((code >> 16) & 0x0F) };
}

} // namespace lcs::ui
