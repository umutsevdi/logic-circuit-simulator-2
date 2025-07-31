#include "core.h"
#include "io.h"
#include "ui/layout.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui {

static void gates(const std::vector<GateNode>&);
static void inputs(const std::vector<InputNode>&);
static void outputs(const std::vector<OutputNode>&);
static void components(const std::vector<ComponentNode>&);

void DebugWindow(NRef<Scene> scene)
{
    if (ImGui::Begin("Scene Debug", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking)) {
        if (scene != nullptr) {
            if (ImGui::CollapsingHeader(
                    "Meta", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::BulletText("Name: %s", scene->name.begin());
                ImGui::BulletText(
                    "Description: %s", scene->description.begin());
                ImGui::BulletText("Author: %s", scene->author.begin());
                ImGui::BulletText("Version: %d", scene->version);
            }
            if (ImGui::CollapsingHeader("Dependencies")) {
                for (auto& s : scene->dependencies) {
                    ImGui::BulletText("%s", s.c_str());
                }
            }

            if (ImGui::CollapsingHeader(
                    "Gates", ImGuiTreeNodeFlags_DefaultOpen)) {
                gates(scene->_gates);
            }
            if (ImGui::CollapsingHeader(
                    "Inputs", ImGuiTreeNodeFlags_DefaultOpen)) {
                inputs(scene->_inputs);
            }
            if (ImGui::CollapsingHeader(
                    "Outputs", ImGuiTreeNodeFlags_DefaultOpen)) {
                outputs(scene->_outputs);
            }
            if (ImGui::CollapsingHeader(
                    "Components", ImGuiTreeNodeFlags_DefaultOpen)) {
                components(scene->_components);
            }
        }
    }
    ImGui::End();
}
static void gates(const std::vector<GateNode>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i].is_null()) {
            ImGui::BulletText("%zu (deleted)", i);
            return;
        }
        if (ImGui::TreeNode(std::to_string(i).c_str())) {
            ImGui::BulletText("Type: %s", to_str<GateNode::Type>(v[i].type()));
            ImGui::BulletText("Input Count: %zu", v[i].inputs.size());
            ImGui::TreePop();
        };
    }
}
static void inputs(const std::vector<InputNode>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i].is_null()) {
            ImGui::BulletText("%zu (deleted)", i);
            return;
        }
        if (ImGui::TreeNode(std::to_string(i).c_str())) {
            ImGui::TreePop();
        }
    }
}
static void outputs(const std::vector<OutputNode>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i].is_null()) {
            ImGui::BulletText("%zu (deleted)", i);
            return;
        }
        if (ImGui::TreeNode(std::to_string(i).c_str())) {
            ImGui::TreePop();
        }
    }
}
static void components(const std::vector<ComponentNode>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i].is_null()) {
            ImGui::BulletText("%zu (deleted)", i);
            return;
        }
        if (ImGui::TreeNode(std::to_string(i).c_str())) {
            ImGui::TreePop();
        }
    }
}
} // namespace lcs::ui
