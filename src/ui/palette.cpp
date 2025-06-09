#include <imgui.h>
#include <imnodes.h>

#include "core.h"
#include "io.h"
#include "ui/layout.h"

namespace lcs::ui {
static bool is_dragging = false;
static Node dragged_node;
void Palette()
{
    auto io = ImGui::GetIO();

    auto scene = io::scene::get();

    ImGui::Begin("Node Palette", nullptr, ImGuiWindowFlags_NoResize);
    if (scene == nullptr) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Input")) {
        dragged_node = scene->add_node<InputNode>();
        is_dragging  = true;
    }
    if (ImGui::Button("Timer")) {
        dragged_node = scene->add_node<InputNode>(1.0f);
        is_dragging  = true;
    }
    if (ImGui::Button("Output")) {
        dragged_node = scene->add_node<OutputNode>();
        is_dragging  = true;
    }
    if (ImGui::Button("NOT Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::NOT);
        is_dragging  = true;
    }
    if (ImGui::Button("AND Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::AND);
        is_dragging  = true;
    }
    if (ImGui::Button("OR Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::OR);
        is_dragging  = true;
    }
    if (ImGui::Button("XOR Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::XOR);
        is_dragging  = true;
    }
    if (ImGui::Button("NAND Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::NAND);
        is_dragging  = true;
    }
    if (ImGui::Button("NOR Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::NOR);
        is_dragging  = true;
    }
    if (ImGui::Button("XNOR Gate")) {
        dragged_node = scene->add_node<GateNode>(GateType::XNOR);
        is_dragging  = true;
    }
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) { }
    if (is_dragging) {
        L_INFO(dragged_node);
        ImNodes::SetNodeGridSpacePos(
            dragged_node.numeric(), ImGui::GetCursorPos());
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            is_dragging  = false;
            dragged_node = 0;
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            scene->remove_node(dragged_node);
            is_dragging  = false;
            dragged_node = 0;
        }
    }
    if (scene == nullptr) {
        ImGui::EndDisabled();
    }

    ImGui::End();
}
} // namespace lcs::ui
