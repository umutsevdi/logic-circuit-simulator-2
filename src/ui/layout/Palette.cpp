#include "core.h"
#include "io.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/layout.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui {
static bool is_dragging = false;
static Node dragged_node;
void Palette(NRef<Scene> scene)
{
    if (!user_data.palette) {
        return;
    }
    auto io = ImGui::GetIO();

    if (ImGui::Begin("Palette", &user_data.palette)) {
        ImGui::BeginDisabled(scene == nullptr);
        if (ImGui::CollapsingHeader("Gates", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginTable("##GatesPalette", 2);
            ImGui::TableSetupColumn("##G1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::NextColumn();
            ImGui::TableSetupColumn("##G2", ImGuiTableColumnFlags_WidthStretch);
            TablePair(
                if (ImGui::Button("Input")) {
                    dragged_node = scene->add_node<InputNode>();
                    is_dragging  = true;
                },
                if (ImGui::Button("Output")) {
                    dragged_node = scene->add_node<OutputNode>();
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("Timer")) {
                    dragged_node = scene->add_node<InputNode>(1.0f);
                    is_dragging  = true;
                },
                if (ImGui::Button("NOT Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::NOT);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("AND Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::AND);
                    is_dragging  = true;
                },
                if (ImGui::Button("NAND Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::NAND);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("OR Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::OR);
                    is_dragging  = true;
                },
                if (ImGui::Button("NOR Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::NOR);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("XOR Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::XOR);
                    is_dragging  = true;
                },
                if (ImGui::Button("XNOR Gate")) {
                    dragged_node = scene->add_node<GateNode>(GateType::XNOR);
                    is_dragging  = true;
                });
            ImGui::EndTable();
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) { }
        if (is_dragging) {
            L_INFO("%s", dragged_node.to_str());
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
        ImGui::EndDisabled();
    }
    ImGui::End();
}
} // namespace lcs::ui
