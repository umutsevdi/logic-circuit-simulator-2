#include <cmath>

#include <imgui.h>
#include <imnodes.h>

#include "common.h"
#include "core.h"
#include "ui/nodes.h"
#include "ui/util.h"

namespace lcs::ui {

void _sync_position(NRef<BaseNode> node, bool has_changes)
{
    uint32_t node_id = node->id().numeric();
    if (has_changes) {
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
    uint32_t compin  = Node { 0, COMPONENT_INPUT }.numeric();
    uint32_t compout = Node { 0, COMPONENT_OUTPUT }.numeric();

    ImNodes::BeginNode(compin);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Component Input");
    ImNodes::EndNodeTitleBar();
    for (size_t i = 0; i < node->inputs.size(); i++) {
        ImNodes::BeginOutputAttribute(encode_pair(node->get_input(i), 0, true),
            to_shape(node->inputs[i].size() > 0, true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();

    ImNodes::BeginNode(compout);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Component Output");
    ImNodes::EndNodeTitleBar();
    for (size_t i = 0; i < node->outputs.size(); i++) {
        ImNodes::BeginInputAttribute(encode_pair(node->get_output(i), 0, false),
            to_shape(node->outputs[i] != 0, true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();
}

template <>
NodeView<InputNode>::NodeView(NRef<InputNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s %u", node->is_timer() ? "Timer " : "Input ", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(encode_pair(node->id(), 0, true),
        to_shape(node->output.size() > 0, false));
    if (node->is_timer()) {
        float freq_value = node->_freq.value();
        ImGui::PushItemWidth(60);
        if (ImGui::InputFloat("Hz", &freq_value, 0.25f, 1.0f, "%.2f")) {
            freq_value = std::max(
                0.25f, std::min(std::round(freq_value * 4.0f) / 4.0f, 10.0f));
            if (freq_value != node->_freq.value()) {
                node->_freq = freq_value;
                io::scene::notify_change();
            }
        }
        ImGui::PopItemWidth();
    } else {
        ToggleButton(&node);
    }
    ImGui::SameLine();
    ImGui::Text("%d", 0);
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

template <>
NodeView<OutputNode>::NodeView(NRef<OutputNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Output %u", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(encode_pair(node->id(), 0, false),
        to_shape(node->is_connected(), false));
    ImGui::Text("%d", 0);
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
}

template <> NodeView<GateNode>::NodeView(NRef<GateNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->get_base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s Gate %u", GateType_to_str(node->type()), node->id().id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node->inputs.size(); i++) {
        if (i == node->inputs.size() / 2) {
            ImNodes::BeginOutputAttribute(encode_pair(node->id(), 0, true),
                to_shape(node->output.size() > 0, false));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::CalcTextSize("         ").x);
            ImGui::Text("0");
            ImNodes::EndOutputAttribute();
        }
        ImNodes::BeginInputAttribute(encode_pair(node->id(), i, false),
            to_shape(node->is_connected(), true));
        ImGui::Text("%zu", i);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

template <> NodeView<ComponentNode>::NodeView(NRef<ComponentNode>, bool) { }

} // namespace lcs::ui
