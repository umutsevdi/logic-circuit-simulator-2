#include "io.h"
#include "ui/components.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui {

void NodeEditor(NRef<Scene> scene)
{
    if (ImGui::BeginChild("NodeEditor")) {
        ImNodes::BeginNodeEditor();
        bool has_changes = io::scene::has_changes();
        if (scene->component_context.has_value()) {
            NodeView<ComponentContext>(
                &scene->component_context.value(), has_changes);
        }
        for (auto& out : scene->_inputs) {
            NodeView<InputNode>(&out.second, has_changes);
        }
        for (auto& out : scene->_outputs) {
            NodeView<OutputNode>(&out.second, has_changes);
        }
        for (auto& out : scene->_gates) {
            NodeView<GateNode>(&out.second, has_changes);
        }
        for (auto& out : scene->_components) {
            NodeView<ComponentNode>(&out.second, has_changes);
        }
        for (auto& r : scene->_relations) {
            ImNodes::PushColorStyle(ImNodesCol_Link,
                r.second.value == State::TRUE ? IM_COL32(0, 255, 100, 255)
                    : r.second.value == State::FALSE
                    ? IM_COL32(255, 0, 100, 255)
                    : IM_COL32(55, 55, 55, 255));
            ImNodes::Link(r.first,
                encode_pair(r.second.from_node, r.second.from_sock, true),
                encode_pair(r.second.to_node, r.second.to_sock, false));
            ImNodes::PopColorStyle();
        }
        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();
        {
            int linkid = 0;
            if (ImNodes::IsLinkHovered(&linkid)) {
                if (auto r = scene->get_rel(linkid);
                    ImGui::BeginTooltip() && r != nullptr) {
                    ImGui::PushFont(
                        get_font(FontFlags::BOLD | FontFlags::NORMAL));
                    ImGui::Text("%s",
                        std::string { "Relation " + std::to_string(linkid) }
                            .c_str());
                    ImGui::Separator();
                    ImGui::PopFont();
                    ImGui::PushFont(get_font(FontFlags::NORMAL));
                    Field("From");
                    ImGui::SameLine();
                    NodeTypeTitle(r->from_node, r->from_sock);
                    Field("To");
                    ImGui::SameLine();
                    NodeTypeTitle(r->to_node, r->to_sock);
                    Field("Value");
                    ImGui::SameLine();
                    ToggleButton(r->value);
                    ImGui::PopFont();
                    ImGui::EndTooltip();
                }
            }

            int nodeid_encoded = 0;
            if (ImNodes::IsNodeHovered(&nodeid_encoded)) {
                Node nodeid { static_cast<uint16_t>(0xFFFF & nodeid_encoded),
                    (NodeType)(nodeid_encoded >> 16) };
                if (NRef<BaseNode> n = scene->get_base(nodeid);
                    ImGui::BeginTooltip() && n != nullptr) {
                    ImGui::PushFont(
                        get_font(FontFlags::BOLD | FontFlags::NORMAL));
                    NodeTypeTitle((Node)nodeid);
                    ImGui::Separator();
                    ImGui::PopFont();
                    ImGui::PushFont(get_font(FontFlags::NORMAL));
                    Field("Position");
                    ImGui::SameLine();
                    ImGui::Text("(%d, %d)", n->point.x, n->point.y);
                    Field("Connected");
                    ImGui::SameLine();
                    ImGui::Text("%s", n->is_connected() ? "true" : "false");
                    Field("Value");
                    ImGui::SameLine();
                    if (nodeid.type != NodeType::COMPONENT) {
                        ToggleButton(n->get());
                    } else {
                        auto comp = scene->get_node<ComponentNode>(nodeid);
                        ImGui::Text("(");
                        ImGui::SameLine();
                        for (size_t i = 0; i < comp->inputs.size(); i++) {
                            ToggleButton(comp->get(i));
                            ImGui::SameLine();
                        }
                        ImGui::Text(")");
                    }

                    ImGui::PopFont();
                    ImGui::EndTooltip();
                } else {
                    ImGui::EndTooltip();
                }
            }

            int pin_id = 0;
            if (ImNodes::IsPinHovered(&pin_id)) {
                bool is_out = 0;
                sockid sock = 0;
                Node nodeid = decode_pair(pin_id, &sock, &is_out);
                if (ImGui::BeginTooltip()) {
                    sock = nodeid.type == COMPONENT_INPUT
                            || nodeid.type == COMPONENT_OUTPUT
                        ? nodeid.id
                        : sock;
                    ImGui::PushFont(
                        get_font(FontFlags::BOLD | FontFlags::NORMAL));
                    ImGui::Text(
                        "%s Socket %u", is_out ? "Output" : "Input", sock);
                    ImGui::Separator();
                    ImGui::PopFont();
                    ImGui::PushFont(get_font(FontFlags::NORMAL));
                    Field("Owner");
                    ImGui::SameLine();
                    NodeTypeTitle(nodeid);
                    if (is_out) {
                        Field("Value");
                        ImGui::SameLine();
                        if (nodeid.type == COMPONENT_INPUT
                            || nodeid.type == COMPONENT_OUTPUT) {
                            ToggleButton(
                                scene->component_context->get_value(nodeid));
                        } else {
                            ToggleButton(scene->get_base(nodeid)->get(sock));
                        }
                    }
                    ImGui::PopFont();
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
                    io::scene::notify_change();
                }
            };
        }
        ImGui::EndChild();
    }
}

} // namespace lcs::ui
