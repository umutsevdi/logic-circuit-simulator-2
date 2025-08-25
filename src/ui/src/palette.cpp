#include <imnodes.h>
#include "components.h"
#include "configuration.h"
#include "core.h"
#include "ui.h"

namespace lcs::ui::layout {
static bool is_dragging = false;
static Node dragged_node;
void Palette(NRef<Scene> scene)
{
    if (!user_data.palette) {
        return;
    }
    auto io = ImGui::GetIO();

    std::string title = std::string { _("Palette") } + "###Palette";
    if (ImGui::Begin(title.c_str(), &user_data.palette)) {
        HINT(nullptr, _("Palette"),
            _("A collection of available nodes that you can add to your\n"
              "project. Select a node from this panel to instantly place\n"
              "it into the editor."));
        ImGui::BeginDisabled(scene == nullptr);
        if (ImGui::CollapsingHeader(
                _("Gates"), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginTable("##GatesPalette", 2);
            ImGui::TableSetupColumn("##G1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::NextColumn();
            ImGui::TableSetupColumn("##G2", ImGuiTableColumnFlags_WidthStretch);
            TablePair(
                if (ImGui::Button(_("Input"))) {
                    dragged_node = scene->add_node<Input>();
                    is_dragging  = true;
                },
                if (ImGui::Button(_("Output"))) {
                    dragged_node = scene->add_node<Output>();
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button(_("Timer"))) {
                    dragged_node
                        = scene->add_node<Input>(static_cast<sockid>(10));
                    is_dragging = true;
                },
                if (ImGui::Button(_("NOT Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NOT);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button(_("AND Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::AND);
                    is_dragging  = true;
                },
                if (ImGui::Button(_("NAND Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NAND);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button(_("OR Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::OR);
                    is_dragging  = true;
                },
                if (ImGui::Button(_("NOR Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::NOR);
                    is_dragging  = true;
                });
            TablePair(
                if (ImGui::Button(_("XOR Gate"))) {
                    dragged_node = scene->add_node<Gate>(Gate::Type::XOR);
                    is_dragging  = true;
                },
                if (ImGui::Button(_("XNOR Gate"))) {
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
