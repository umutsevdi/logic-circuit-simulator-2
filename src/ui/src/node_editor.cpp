#include "components.h"
#include "core.h"
#include "ui.h"
#include <imgui.h>
#include <imnodes.h>
#include <cmath>

namespace lcs::ui::layout {
static void _show_node(NRef<Input> base_node, uint16_t id, bool has_changes);
static void _show_node(NRef<Output> base_node, uint16_t id, bool has_changes);
static void _show_node(NRef<Gate> base_node, uint16_t id, bool has_changes);
static void _show_node(
    NRef<Component> base_node, uint16_t id, bool has_changes);
static void _show_node(
    NRef<ComponentContext> base_node, uint16_t id, bool has_changes);

void _sync_position(NRef<BaseNode> node, Node id, bool has_changes)
{
    uint32_t node_id = id.numeric();
    if (has_changes) {
        ImNodes::SetNodeGridSpacePos(
            node_id, { (float)node->point.x, (float)node->point.y });
    } else {
        auto pos = ImNodes::GetNodeGridSpacePos(node_id);
        pos      = { std::floor(pos.x), std::floor(pos.y) };
        ImNodes::SetNodeGridSpacePos(node_id, pos);
        if (pos.x != node->point.x || pos.y != node->point.y) {
            node->point
                = { static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y) };
            tabs::notify();
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

void _show_node(NRef<ComponentContext> node, uint16_t, bool)
{
    uint32_t compin  = Node { 0, Node::COMPONENT_INPUT }.numeric();
    uint32_t compout = Node { 0, Node::COMPONENT_OUTPUT }.numeric();

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

void _show_node(NRef<Input> node, uint16_t id, bool has_changes)
{
    uint32_t nodeid = Node { id, Node::Type::INPUT }.numeric();
//    L_INFO("%d %d", nodeid, encode_pair(id, 0, true));
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), id, has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s %u", node->is_timer() ? "Timer " : "Input ", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(
        encode_pair(id, 0, true), to_shape(node->output.size() > 0, false));
    if (node->is_timer()) {
        ImGui::PushItemWidth(60);
        float freq_value = static_cast<float>(node->_freq) / 10.f;
        if (ImGui::SliderFloat("Hz", &freq_value, 0.1f, 5.0f, "%.1f")) {
            if (freq_value != node->_freq) {
                node->_freq = freq_value * 10;
                tabs::notify();
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

void _show_node(NRef<Output> node, uint16_t id, bool has_changes)
{
    uint32_t nodeid = Node { id, Node::Type::OUTPUT }.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), id, has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Output %u", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(
        encode_pair(id, 0, false), to_shape(node->is_connected(), false));
    ImGui::Text("%d", 1);
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
}

void _show_node(NRef<Gate> node, uint16_t id, bool has_changes)
{
    uint32_t nodeid = Node { id, Node::Type::GATE }.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), id, has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s Gate %u", to_str<Gate::Type>(node->type()), id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node->inputs.size(); i++) {
        if (i == node->inputs.size() / 2) {
            ImNodes::BeginOutputAttribute(encode_pair(id, 0, true),
                to_shape(node->output.size() > 0, false));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::CalcTextSize("         ").x);
            ImGui::Text("1");
            ImNodes::EndOutputAttribute();
        }
        ImNodes::BeginInputAttribute(
            encode_pair(id, i, false), to_shape(node->is_connected(), true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

void _show_node(NRef<Component> node, uint16_t id, bool has_changes)
{
    uint32_t nodeid = Node { id, Node::Type::COMPONENT }.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node->base(), id, has_changes);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Component Node %u", id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node->inputs.size(); i++) {
        if (i == node->inputs.size() / 2) {
            for (size_t j = 0; j < node->outputs.size(); j++) {
                ImNodes::BeginOutputAttribute(encode_pair(id, j, true),
                    to_shape(!node->outputs[j].empty(), false));
                ImGui::SetCursorPosX(ImGui::GetCursorPosX()
                    + ImGui::CalcTextSize("         ").x);
                ImGui::Text("%zu", j + 1);
                ImNodes::EndOutputAttribute();
            }
        }
        ImNodes::BeginInputAttribute(
            encode_pair(id, i, false), to_shape(node->is_connected(), true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

void NodeEditor(NRef<Scene> scene)
{
    if (ImGui::Begin("Editor", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoNavFocus)) {
        ImNodes::BeginNodeEditor();
        if (scene == nullptr) {
            ImNodes::EndNodeEditor();
            ImGui::End();
            return;
        }
        const LcsTheme& style = get_active_style();
        bool has_changes      = tabs::is_changed();
        if (scene->component_context.has_value()) {
            _show_node(&scene->component_context.value(), 0, has_changes);
        }
        for (size_t i = 0; i < scene->_inputs.size(); i++) {
            if (!scene->_inputs[i].is_null()) {
                _show_node(&scene->_inputs[i], i, has_changes);
            }
        }
        for (size_t i = 0; i < scene->_outputs.size(); i++) {
            if (!scene->_outputs[i].is_null()) {
                _show_node(&scene->_outputs[i], i, has_changes);
            }
        }
        for (size_t i = 0; i < scene->_gates.size(); i++) {
            if (!scene->_gates[i].is_null()) {
                _show_node(&scene->_gates[i], i, has_changes);
            }
        }
        for (size_t i = 0; i < scene->_components.size(); i++) {
            if (!scene->_components[i].is_null()) {
                _show_node(&scene->_components[i], i, has_changes);
            }
        }
        for (auto& r : scene->_relations) {

            ImNodes::PushColorStyle(ImNodesCol_Link,
                r.second.value == State::TRUE ? ImGui::GetColorU32(style.green)
                    : r.second.value == State::FALSE
                    ? ImGui::GetColorU32(style.red)
                    : ImGui::GetColorU32(style.black_bright));
            ImNodes::Link(r.first,
                encode_pair(r.second.from_node, r.second.from_sock, true),
                encode_pair(r.second.to_node, r.second.to_sock, false));
            ImNodes::PopColorStyle();
        }
        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();

        int linkid = 0;
        if (ImNodes::IsLinkHovered(&linkid)) {
            if (auto r = scene->get_rel(linkid);
                r != nullptr && ImGui::BeginTooltip()) {
                SubSection("Relation %d", linkid);
                Field("From");
                ImGui::SameLine();
                NodeTypeTitle(r->from_node, r->from_sock);
                Field("To");
                ImGui::SameLine();
                NodeTypeTitle(r->to_node, r->to_sock);
                Field("Value");
                ImGui::SameLine();
                ToggleButton(r->value);
                EndSection();
                ImGui::EndTooltip();
            }
        }

        int nodeid_encoded = 0;
        if (ImNodes::IsNodeHovered(&nodeid_encoded)) {
            Node nodeid { static_cast<uint16_t>(0xFFFF & nodeid_encoded),
                (Node::Type)(nodeid_encoded >> 16) };
            if (NRef<BaseNode> n = scene->get_base(nodeid);
                n != nullptr && ImGui::BeginTooltip()) {
                SubSection("Node %s@%d", to_str<Node::Type>(nodeid.type),
                    nodeid.index);
                Field("Position");
                ImGui::SameLine();
                ImGui::Text("(%d, %d)", n->point.x, n->point.y);
                Field("Connected");
                ImGui::SameLine();
                ImGui::Text("%s", n->is_connected() ? "true" : "false");
                Field("Value");
                ImGui::SameLine();
                if (nodeid.type != Node::Type::COMPONENT) {
                    ToggleButton(n->get());
                } else {
                    auto comp = scene->get_node<Component>(nodeid);
                    ImGui::Text("(");
                    ImGui::SameLine();
                    for (size_t i = 0; i < comp->outputs.size(); i++) {
                        ToggleButton(comp->get(i));
                        ImGui::SameLine();
                    }
                    ImGui::Text(")");
                }
                EndSection();
                ImGui::EndTooltip();
            }
        }

        int pin_id = 0;
        if (ImNodes::IsPinHovered(&pin_id)) {
            bool is_out = false;
            sockid sock = 0;
            Node nodeid = decode_pair(pin_id, &sock, &is_out);
            if (ImGui::BeginTooltip()) {
                sock = nodeid.type == Node::Type::COMPONENT_INPUT
                        || nodeid.type == Node::Type::COMPONENT_OUTPUT
                    ? nodeid.index
                    : sock;
                SubSection("%s Socket %u", is_out ? "Output" : "Input", sock);
                Field("Owner");
                ImGui::SameLine();
                NodeTypeTitle(nodeid);
                if (is_out) {
                    Field("Value");
                    ImGui::SameLine();
                    if (nodeid.type == Node::Type::COMPONENT_INPUT
                        || nodeid.type == Node::Type::COMPONENT_OUTPUT) {
                        ToggleButton(
                            scene->component_context->get_value(nodeid));
                    } else {
                        ToggleButton(scene->get_base(nodeid)->get(sock));
                    }
                }
                EndSection();
                ImGui::EndTooltip();
            }
        }
        int start_pin_id = 0;
        int end_pin_id   = 0;
        if (ImNodes::IsLinkCreated(&start_pin_id, &end_pin_id)) {
            sockid from_sock = 0, to_sock = 0;
            Node from = decode_pair(start_pin_id, &from_sock);
            Node to   = decode_pair(end_pin_id, &to_sock);
            if (!scene->connect(to, to_sock, from, from_sock)) {
                tabs::notify();
            }
        };
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)
            && ImNodes::NumSelectedNodes() > 0) {
            size_t node_s = ImNodes::NumSelectedNodes();
            size_t link_s = ImNodes::NumSelectedLinks();
            if (node_s && link_s) {
                L_INFO("Context for Nodes && Links");

            } else if (node_s) {
                L_INFO("Context for Nodes");
            } else if (link_s) {
                L_INFO("Context for Links");
            }
        }
    }
    ImGui::End();
}

} // namespace lcs::ui::layout
