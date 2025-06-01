#include <imgui.h>

#include "ui/nodes.h"
#include "ui.h"

namespace lcs::ui {

static inline void _state_toggle_button(NRef<InputNode> node)
{
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    switch (node->get()) {
    case state_t::TRUE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("TRUE")) { node->toggle(); };
        ImGui::PopStyleColor();
        break;
    case state_t::FALSE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("FALSE")) { node->toggle(); };
        break;
    case state_t::DISABLED:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::Button("DISABLED");
        break;
    default: break;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
}

static inline ImNodesPinShape_ to_shape(bool value, bool is_input)
{
    if (is_input) {
        return value ? ImNodesPinShape_TriangleFilled
                     : ImNodesPinShape_Triangle;
    }
    return value ? ImNodesPinShape_QuadFilled : ImNodesPinShape_Quad;
}

template <> void NodeView<InputNode>::_set_inputs(void) { }
template <> void NodeView<InputNode>::_set_outputs(void)
{
    ImNodes::BeginOutputAttribute(_hash_node_sock_pair(node->id(), 0),
        to_shape(node->output.size() > 0, false));
    ImGui::TextUnformatted(std::to_string(0).c_str());
    ImNodes::EndOutputAttribute();
}

template <> void NodeView<OutputNode>::_set_inputs(void)
{
    ImNodes::BeginInputAttribute(_hash_node_sock_pair(node->id(), 0),
        to_shape(node->is_connected(), true));
    ImGui::TextUnformatted("0");
    ImNodes::EndInputAttribute();
}
template <> void NodeView<OutputNode>::_set_outputs(void) { }

template <> void NodeView<GateNode>::_set_inputs(void)
{
    for (size_t i = 0; i < node->inputs.size(); i++) {
        ImNodes::BeginInputAttribute(_hash_node_sock_pair(node->id(), i),
            to_shape(node->inputs[i] != 0, true));
        ImGui::TextUnformatted(
            (std::to_string(i) + " ->" + std::to_string(node->inputs[i]))
                .c_str());
        ImNodes::EndInputAttribute();
        ImGui::Text(node->is_connected() ? "true" : "false");
    }
}
template <> void NodeView<GateNode>::_set_outputs(void)
{
    ImNodes::BeginOutputAttribute(_hash_node_sock_pair(node->id(), 0),
        to_shape(node->output.size() > 0, false));
    ImGui::TextUnformatted(std::to_string(0).c_str());
    ImNodes::EndOutputAttribute();
}

template <> void NodeView<ComponentNode>::_set_inputs(void) { }
template <> void NodeView<ComponentNode>::_set_outputs(void) { }

template <> void NodeView<ComponentNode>::_show_node_window(void) { }
template <> void NodeView<GateNode>::_show_node_window(void) { }
template <> void NodeView<OutputNode>::_show_node_window(void) { }
template <> void NodeView<InputNode>::_show_node_window(void)
{
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 app_size = io.DisplaySize;
    ImGui::Begin("ShowSelectDisplay", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::SetWindowSize(ImVec2 { app_size.x * 0.15f, app_size.y * 0.2f });
    ImGui::SetWindowPos(ImVec2 { app_size.x * 0.85f, app_size.y * 0.8f });
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::LARGE));
    ImGui::Text("Node %s@%ul", node_to_str(node->id().type), node->id().id);
    ImGui::Separator();
    ImGui::PopFont();
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Id:");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::Text("%ul", node->id().id);
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Value:");
    ImGui::PopFont();
    ImGui::SameLine();

    _state_toggle_button(&node);

    ImGui::Text("%ul", node->id().id);

    //  node_to_title(r->from_node, r->from_sock);
    //  ImGui::PushFont(get_font(font_flags_t::NORMAL));
    //  ImGui::TextColored(ImVec4(200, 200, 0, 255), "To:");
    //  ImGui::PopFont();
    //  ImGui::SameLine();
    //  node_to_title(r->to_node, r->to_sock);
    ImGui::End();
}

} // namespace lcs::ui
  //
