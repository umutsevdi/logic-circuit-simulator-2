#include <cmath>
#include <imgui.h>

#include "common.h"
#include "core.h"
#include "imnodes.h"
#include "ui.h"
#include "ui/nodes.h"

namespace lcs::ui {

void _sync_position(NRef<BaseNode> node, bool first_time)
{
    uint32_t node_id = node->id().numeric();
    if (first_time) {
        ImNodes::SetNodeGridSpacePos(
            node_id, { (float)node->point.x, (float)node->point.y });
    } else {
        auto pos = ImNodes::GetNodeGridSpacePos(node_id);
        pos      = { std::floor(pos.x), std::floor(pos.y) };
        ImNodes::SetNodeGridSpacePos(node_id, pos);
        if (pos.x != node->point.x || pos.y != node->point.y) {
            node->point = { (int)pos.x, (int)pos.y };
            lcs::io::scene::notify_change();
        }
    }
}

static inline void _Stateoggle_button(NRef<InputNode> node)
{
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
    switch (node->get()) {
    case State::TRUE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("TRUE ")) {
            node->toggle();
        };
        ImGui::PopStyleColor();
        break;
    case State::FALSE:
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("FALSE")) {
            node->toggle();
        };
        break;
    case State::DISABLED:
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

template <>
NodeView<ComponentContext>::NodeView(NRef<ComponentContext> node, bool)
{
    uint32_t compin  = hash_pair(node->get_input(0), 0, true);
    uint32_t compout = hash_pair(node->get_output(0), 0, false);

    ImNodes::BeginNode(compin);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Component Input");
    ImNodes::EndNodeTitleBar();
    for (auto& in : node->inputs) {
        ImNodes::BeginOutputAttribute(
            hash_pair(node->get_input(in.first), 0, true),
            to_shape(in.second.size() > 0, true));
        ImGui::Text("%u", in.first);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();

    ImNodes::BeginNode(compout);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Component Output");
    ImNodes::EndNodeTitleBar();
    for (auto& out : node->outputs) {
        ImNodes::BeginInputAttribute(
            hash_pair(node->get_output(out.first), 0, false),
            to_shape(out.second != 0, true));
        ImGui::Text("%u", out.first);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();

    //  auto window_size = ImGui::GetWindowSize();
    //  ImNodes::SetNodeGridSpacePos(compin, { 0, window_size.y / 2 });
    //  ImNodes::SetNodeGridSpacePos(
    //      compout, { window_size.x - 60, window_size.y / 2 });

    //  if (ImNodes::IsNodeSelected(compin)) { _show_node_window(); }
    //  if (ImNodes::IsNodeSelected(compout)) { _show_node_window(); }
}

template <> NodeView<InputNode>::NodeView(NRef<InputNode> node, bool first_time)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), first_time);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s %u", node->is_timer() ? "Timer " : "Input ", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(hash_pair(node->id(), 0, true),
        to_shape(node->output.size() > 0, false));
    if (node->is_timer()) {
        int freq_value = node->_freq.value();
        ImGui::PushItemWidth(60);
        if (ImGui::InputInt("Hz", &freq_value)) {
            if (freq_value != (int)node->_freq.value()) {
                node->_freq = std::max(0, std::min(freq_value, 60));
            }
        }
        ImGui::PopItemWidth();
    } else {
        _Stateoggle_button(&node);
    }
    ImGui::SameLine();
    ImGui::Text("%d", 0);
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
    if (ImNodes::IsNodeSelected(nodeid)) {
        _show_node_window();
    }
}

template <>
NodeView<OutputNode>::NodeView(NRef<OutputNode> node, bool first_time)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), first_time);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Output %u", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(
        hash_pair(node->id(), 0, false), to_shape(node->is_connected(), false));
    ImGui::Text("%d", 0);
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
    if (ImNodes::IsNodeSelected(nodeid)) {
        _show_node_window();
    }
}

template <> NodeView<GateNode>::NodeView(NRef<GateNode> node, bool first_time)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), first_time);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s Gate %u", GateType_to_str(node->type()), node->id().id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node->inputs.size(); i++) {
        if (i == node->inputs.size() / 2) {
            ImNodes::BeginOutputAttribute(hash_pair(node->id(), 0, true),
                to_shape(node->output.size() > 0, false));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::CalcTextSize("         ").x);
            ImGui::Text("0");
            ImNodes::EndOutputAttribute();
        }
        ImNodes::BeginInputAttribute(hash_pair(node->id(), i, false),
            to_shape(node->is_connected(), true));
        ImGui::Text("%zu", i);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
    if (ImNodes::IsNodeSelected(nodeid)) {
        _show_node_window();
    }
}

template <> NodeView<ComponentNode>::NodeView(NRef<ComponentNode>, bool) { }

/*template <> void NodeView<InputNode>::_show_node_window(void)
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
    ImGui::Text("Node %s@%ul", NodeType_to_str(node->id().type), node->id().id);
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

    _Stateoggle_button(&node);

    ImGui::Text("%ul", node->id().id);

    //  NodeType_to_title(r->from_node, r->from_sock);
    //  ImGui::PushFont(get_font(font_flags_t::NORMAL));
    //  ImGui::TextColored(ImVec4(200, 200, 0, 255), "To:");
    //  ImGui::PopFont();
    //  ImGui::SameLine();
    //  NodeType_to_title(r->to_node, r->to_sock);
    ImGui::End();
}
*/

} // namespace lcs::ui
  //
