#include "io.h"
#include "ui/components.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <cmath>

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
            io::scene::notify_change();
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

template <> void NodeView<ComponentContext>(NRef<ComponentContext> node, bool)
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

template <> void NodeView<InputNode>(NRef<InputNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s %u", node->is_timer() ? "Timer " : "Input ", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(encode_pair(node->id(), 0, true),
        to_shape(node->output.size() > 0, false));
    if (node->is_timer()) {
        float freq_value = node->_freq.value();
        ImGui::PushItemWidth(60);
        if (ImGui::SliderFloat("Hz", &freq_value, 0.1f, 5.0f, "%.1f")) {
            if (freq_value != node->_freq.value()) {
                node->_freq = freq_value;
                io::scene::notify_change();
            }
        }
        ImGui::PopItemWidth();
    } else {
        State old = node->get();
        if (State t = ToggleButton(old, true); t != old) {
            node->toggle();
        }
    }
    ImGui::SameLine();
    ImGui::Text("%d", 1);
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

template <> void NodeView<OutputNode>(NRef<OutputNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Output %u", node->id().id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(encode_pair(node->id(), 0, false),
        to_shape(node->is_connected(), false));
    ImGui::Text("%d", 1);
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
}

template <> void NodeView<GateNode>(NRef<GateNode> node, bool has_changes)
{
    uint32_t nodeid = node->id().numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s Gate %u", GateType_to_str(node->type()), node->id().id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node->inputs.size(); i++) {
        if (i == node->inputs.size() / 2) {
            ImNodes::BeginOutputAttribute(encode_pair(node->id(), 0, true),
                to_shape(node->output.size() > 0, false));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::CalcTextSize("         ").x);
            ImGui::Text("1");
            ImNodes::EndOutputAttribute();
        }
        ImNodes::BeginInputAttribute(encode_pair(node->id(), i, false),
            to_shape(node->is_connected(), true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

template <> void NodeView<ComponentNode>(NRef<ComponentNode>, bool) { }

} // namespace lcs::ui
