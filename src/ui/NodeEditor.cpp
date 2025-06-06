
#include "core.h"
#include "imgui.h"
#include "io.h"
#include "ui.h"
#include "ui/nodes.h"
#include "ui/util.h"
#include <imnodes.h>

namespace lcs::ui {
void _value_tooltip(State s);

int hash_pair(Node node, sockid sock, bool is_out)
{
    // WARN: I hope you never reach 24 bit id.
    int x = node.id;
    lcs_assert(node.id < 0xFFFF);
    x |= node.type << 16;
    x |= sock << 20;
    if (is_out) {
        x |= 1 << 21;
    }
    return x;
}

void NodeEditor(NRef<Scene> scene)
{
    if (ImGui::BeginChild("NodeEditor")) {
        ImNodes::BeginNodeEditor();
        bool scene_changed = io::scene::first_frame();
        if (scene->component_context.has_value()) {
            NodeView<ComponentContext> { &scene->component_context.value(),
                scene_changed };
        }
        for (auto& out : scene->_inputs) {
            NodeView<InputNode>(&out.second, scene_changed);
        }
        for (auto& out : scene->_outputs) {
            NodeView<OutputNode>(&out.second, scene_changed);
        }
        for (auto& out : scene->_gates) {
            NodeView<GateNode>(&out.second, scene_changed);
        }
        for (auto& out : scene->_components) {
            NodeView<ComponentNode>(&out.second, scene_changed);
        }
        for (auto& r : scene->_relations) {
            ImNodes::PushColorStyle(ImNodesCol_Link,
                r.second.value == State::TRUE ? IM_COL32(0, 255, 100, 255)
                    : r.second.value == State::FALSE
                    ? IM_COL32(255, 0, 100, 255)
                    : IM_COL32(55, 55, 55, 255));
            if (r.second.from_node.type != COMPONENT_OUTPUT
                && r.second.from_node.type != COMPONENT_INPUT
                && r.second.to_node.type != COMPONENT_OUTPUT
                && r.second.to_node.type != COMPONENT_INPUT) {
                ImNodes::Link(r.first,
                    hash_pair(r.second.from_node, r.second.from_sock, true),
                    hash_pair(r.second.to_node, r.second.to_sock, false));
            } else {
                L_INFO(r.second.from_node << " -> " << r.second.to_node);
            }
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
                        get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
                    ImGui::Text("%s",
                        std::string { "Relation " + std::to_string(linkid) }
                            .c_str());
                    ImGui::Separator();
                    ImGui::PopFont();
                    ImGui::PushFont(get_font(font_flags_t::NORMAL));
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "From:");
                    ImGui::SameLine();
                    NodeTypeTitle(r->from_node, r->from_sock);
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "To:");
                    ImGui::SameLine();
                    NodeTypeTitle(r->to_node, r->to_sock);
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Value:");
                    ImGui::SameLine();
                    _value_tooltip(r->value);
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
                        get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
                    NodeTypeTitle((Node)nodeid);
                    ImGui::Separator();
                    ImGui::PopFont();
                    ImGui::PushFont(get_font(font_flags_t::NORMAL));
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Position:");
                    ImGui::SameLine();
                    ImGui::Text("(%d, %d)", n->point.x, n->point.y);
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Connected:");
                    ImGui::SameLine();
                    ImGui::Text("%s", n->is_connected() ? "true" : "false");
                    if (nodeid.type != NodeType::COMPONENT) {
                        ImGui::TextColored(ImVec4(200, 200, 0, 255), "Value:");
                        ImGui::SameLine();
                        _value_tooltip(n->get());
                    } else {
                        auto comp = scene->get_node<ComponentNode>(nodeid);
                        ImGui::Text("(");
                        ImGui::SameLine();
                        for (size_t i = 0; i < comp->inputs.size(); i++) {
                            _value_tooltip(comp->get(i));
                            ImGui::SameLine();
                        }
                        ImGui::Text(")");
                    }

                    ImGui::PopFont();
                    ImGui::EndTooltip();
                }
            }
        }
        ImGui::EndChild();
    }
}

} // namespace lcs::ui
