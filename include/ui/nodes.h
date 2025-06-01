#pragma once
/*******************************************************************************
 * \file
 * File: src/ui/nodes.hpp
 * Created: 05/30/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <cmath>
#include <cstdint>
#include <imnodes.h>

#include "core.h"
#include "io.h"
namespace lcs::ui {

int _hash_node_sock_pair(node node, sockid sock);

template <typename T> class NodeView {
public:
    NodeView(T* base_node)
        : node { base_node } { };
    NodeView(NodeView&&) = default;
    NodeView() { }

    void show_node(bool first_time)
    {
        uint32_t node_id = node->id().numeric();
        ImNodes::BeginNode(node_id);
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
        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s",
            (std::to_string(node->id().id) + node_to_str(node->id().type))
                .c_str());
        ImNodes::EndNodeTitleBar();
        _set_inputs();
        _set_outputs();
        ImGui::Dummy(ImVec2(80.0f, 45.0f));
        ImGui::Text("-> In");
        ImGui::SameLine();
        ImGui::Text("Out ->");
        ImNodes::EndNode();
        if (ImNodes::IsNodeSelected(node->id().numeric())) {
            _show_node_window();
        }
    }

private:
    NRef<T> node;
    void _show_node_window(void);
    void _set_inputs(void);
    void _set_outputs(void);
};

} // namespace lcs::ui
