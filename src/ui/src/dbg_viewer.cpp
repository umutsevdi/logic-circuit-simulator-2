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
    std::string title
        = std::string { _("Memory Debugger") } + "###MemoryDebugger";
    if (ImGui::Begin(title.c_str(), nullptr,
            ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoNavFocus)) {
        if (scene != nullptr) {
            ImVec2 wsize = ImVec2((ImGui::GetContentRegionAvail().x
                                      - ImGui::GetStyle().ItemSpacing.x)
                    * 0.5f,
                ImGui::GetContentRegionAvail().y);
            ImGui::BeginChild("##Left", wsize);
            if (ImGui::CollapsingHeader(
                    _("Meta"), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::BulletText(_("Name: %s"), scene->name.begin());
                ImGui::BulletText(
                    _("Description: %s"), scene->description.begin());
                ImGui::BulletText(_("Author: %s"), scene->author.begin());
                ImGui::BulletText(_("Version: %d"), scene->version);
            }
            if (ImGui::CollapsingHeader(
                    _("Dependencies"), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto& s : scene->dependencies()) {
                    ImGui::BulletText("%s", s.to_dependency().c_str());
                }
            }
            if (ImGui::CollapsingHeader(
                    _("Raw"), ImGuiTreeNodeFlags_DefaultOpen)) {
                std::vector<uint8_t> raw;
                Error _ = scene->write_to(raw);
                if (!raw.empty()) {
                    std::array<char, 10 * 3 + 1> buf {};
                    for (size_t i = 0; i < raw.size() / 10; i++) {
                        size_t idx = 0;
                        for (auto it = raw.begin() + 10 * i;
                            it != raw.end() && it != raw.begin() + 10 * (i + 1);
                            it++) {
                            std::snprintf(buf.data() + idx, 4, "%02x ", *it);
                            idx += 3;
                        }
                        ImGui::TextUnformatted(buf.begin());
                    }
                }
            }

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("##Right", wsize);
            ImGui::PushID(to_str<Node::Type>(Node::Type::GATE));
            if (ImGui::CollapsingHeader(
                    _("Gates"), ImGuiTreeNodeFlags_DefaultOpen)) {
                _gates(scene->_gates);
            }
            ImGui::PopID();
            ImGui::PushID(to_str<Node::Type>(Node::Type::INPUT));
            if (ImGui::CollapsingHeader(
                    _("Inputs"), ImGuiTreeNodeFlags_DefaultOpen)) {
                _inputs(scene->_inputs);
            }
            ImGui::PopID();
            ImGui::PushID(to_str<Node::Type>(Node::Type::OUTPUT));
            if (ImGui::CollapsingHeader(
                    _("Outputs"), ImGuiTreeNodeFlags_DefaultOpen)) {
                _outputs(scene->_outputs);
            }
            ImGui::PopID();
            ImGui::PushID(to_str<Node::Type>(Node::Type::COMPONENT));
            if (ImGui::CollapsingHeader(
                    _("Components"), ImGuiTreeNodeFlags_DefaultOpen)) {
                _components(scene->_components);
            }
            ImGui::PopID();
            ImGui::EndChild();
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
            ImGui::BulletText("Value: %s", to_str<State>(v[i].get()));
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
            ImGui::BulletText("Type: %s", v[i].is_timer() ? "TIMER" : "INPUT");
            ImGui::BulletText("Value: %s", to_str<State>(v[i].get()));
            if (v[i].is_timer()) {
                ImGui::BulletText("Freq: %f", v[i]._freq / 10.f);
            }
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
            ImGui::BulletText("Value: %s", to_str<State>(v[i].get()));
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
            ImGui::BulletText("DepId: %d", v[i].dep_idx);
            ImGui::BulletText("Input Size: %zu", v[i].inputs.size());
            ImGui::BulletText("Output Size: %zu", v[i].outputs.size());
            ImGui::TreePop();
        }
    }
}
} // namespace lcs::ui::layout
