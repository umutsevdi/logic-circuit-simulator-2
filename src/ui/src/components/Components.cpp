#include "components.h"
#include "io.h"
#include <imnodes.h>

namespace lcs::ui {
void ShowIcon(FontFlags size, const char* icon)
{
    ImGui::PushFont(get_font(FontFlags::ICON | size));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
}

bool PositionSelector(Point& point, const char* prefix)
{
    const static ImVec2 __selector_size = ImGui::CalcTextSize("-000000000000");

    std::string s_prefix = "##";
    s_prefix += prefix;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("x");
    ImGui::SameLine();
    ImGui::PushItemWidth(__selector_size.x);
    bool change_x
        = ImGui::InputInt((s_prefix + "X").c_str(), &point.x, 1.0f, 10.0f, 0);
    ImGui::Text("y");
    ImGui::SameLine();
    bool change_y
        = ImGui::InputInt((s_prefix + "Y").c_str(), &point.y, 1.0f, 10.0f, 0);
    ImGui::PopItemWidth();
    return change_x || change_y;
}

State ToggleButton(State state, bool clickable)
{
    const LcsTheme& style = get_active_style();
    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::SMALL));
    switch (state) {
    case State::TRUE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.green);
            ImGui::PushStyleColor(ImGuiCol_Text, style.fg);
            if (ImGui::Button("TRUE ")) {
                state = FALSE;
            };
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(style.green, "TRUE");
        }
        break;
    case State::FALSE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.red);
            if (ImGui::Button("FALSE")) {
                state = TRUE;
            };
        } else {
            ImGui::TextColored(style.red, "FALSE");
        }
        break;
    case State::DISABLED:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.black_bright);
            ImGui::PushStyleColor(ImGuiCol_Text, style.white_bright);
            ImGui::Button("DISCONNECTED");
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(style.black_bright, "DISCONNECTED");
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
    snprintf(buffer, 256, "%s@%u", to_str<Node::Type>(n.type), n.index);
    if (ImGui::TextLink(buffer)) {
        ImNodes::ClearNodeSelection();
        switch (n.type) {
        case Node::Type::COMPONENT_INPUT:
        case Node::Type::COMPONENT_OUTPUT:
            ImNodes::SelectNode(Node { 0, n.type }.numeric());
            break;
        default: ImNodes::SelectNode(n.numeric()); break;
        }
    };
    ImGui::PopStyleColor();
}

void NodeTypeTitle(Node n, sockid)
{
    ImGui::PushFont(get_font(FontFlags::REGULAR | FontFlags::NORMAL));
    NodeTypeTitle(n);
    ImGui::PopFont();
}
} // namespace lcs::ui
