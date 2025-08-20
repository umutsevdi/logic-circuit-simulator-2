#include "components.h"
#include "configuration.h"
#include "core.h"
#include "ui.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui::layout {
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
                    dragged_node = scene->add_node<Input>();
                    is_dragging  = true;
                },
                if (ImGui::Button("Output")) {
                    dragged_node = scene->add_node<Output>();
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("Timer")) {
                    dragged_node = scene->add_node<Input>(10u);
                    is_dragging  = true;
                },
                if (ImGui::Button("NOT Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NOT);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("AND Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::AND);
                    is_dragging  = true;
                },
                if (ImGui::Button("NAND Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NAND);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("OR Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::OR);
                    is_dragging  = true;
                },
                if (ImGui::Button("NOR Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NOR);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button("XOR Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::XOR);
                    is_dragging  = true;
                },
                if (ImGui::Button("XNOR Gate")) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::XNOR);
                    is_dragging  = true;
                });
            ImGui::EndTable();
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) { }
        if (is_dragging) {
            L_DEBUG("%s", to_str<Node>(dragged_node));
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
} // namespace lcs::ui::layout
