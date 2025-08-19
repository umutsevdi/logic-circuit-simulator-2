#include "core.h"
#include "ui.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui::layout {

static void _gates(const std::vector<Gate>&);
static void _inputs(const std::vector<Input>&);
static void _outputs(const std::vector<Output>&);
static void _components(const std::vector<Component>&);

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
                for (auto& s : scene->dependencies()) {
                    ImGui::BulletText("%s", s.to_dependency().c_str());
                }
            }

            if (ImGui::CollapsingHeader(
                    "Gates", ImGuiTreeNodeFlags_DefaultOpen)) {
                _gates(scene->_gates);
            }
            if (ImGui::CollapsingHeader(
                    "Inputs", ImGuiTreeNodeFlags_DefaultOpen)) {
                _inputs(scene->_inputs);
            }
            if (ImGui::CollapsingHeader(
                    "Outputs", ImGuiTreeNodeFlags_DefaultOpen)) {
                _outputs(scene->_outputs);
            }
            if (ImGui::CollapsingHeader(
                    "Components", ImGuiTreeNodeFlags_DefaultOpen)) {
                _components(scene->_components);
            }
        }
    }
    ImGui::End();
}
static void _gates(const std::vector<Gate>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i].is_null()) {
            ImGui::BulletText("%zu (deleted)", i);
            return;
        }
        if (ImGui::TreeNode(std::to_string(i).c_str())) {
            ImGui::BulletText("Type: %s", to_str<Gate::Type>(v[i].type()));
            ImGui::BulletText("Input Count: %zu", v[i].inputs.size());
            ImGui::TreePop();
        };
    }
}
static void _inputs(const std::vector<Input>& v)
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
static void _outputs(const std::vector<Output>& v)
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
static void _components(const std::vector<Component>& v)
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
} // namespace lcs::ui::layout
