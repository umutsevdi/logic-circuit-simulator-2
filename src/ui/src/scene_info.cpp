#include "IconsLucide.h"
#include "components.h"
#include "configuration.h"
#include "imgui.h"
#include "ui.h"

namespace lcs::ui ::layout {
void SceneInfo(NRef<Scene> scene)
{
    if (!user_data.scene_info) {
        return;
    }
    if (ImGui::Begin("Scene Information", &user_data.scene_info)) {
        ImGui::BeginDisabled(scene == nullptr);
        Section("Scene");
        Field("Scene Name");
        if (scene != nullptr
            && ImGui::InputText("##SceneNameInputText", scene->name.data(),
                scene->name.max_size(), ImGuiInputTextFlags_CharsNoBlank)) {
            tabs::notify();
        };

        Field("Description");
        if (scene != nullptr
            && ImGui::InputTextMultiline("##SceneDescInputText",
                scene->description.data(), scene->description.max_size(),
                ImVec2(ImGui::GetWindowWidth(),
                    ImGui::CalcTextSize("\n\n\n").y))) {
            tabs::notify();
        };

        Field("Version");
        if (scene != nullptr
            && ImGui::InputInt("##SceneVersion", &scene->version)) {
            scene->version = std::max(scene->version, 0);
            tabs::notify();
        };
        EndSection();

        if (scene != nullptr && scene->component_context.has_value()) {
            Section("Component Attributes");
            Field("Input Size");
            size_t input_size  = scene->component_context->inputs.size();
            size_t output_size = scene->component_context->outputs.size();
            ImGui::InputInt("##CompInputSize", (int*)&input_size);
            Field("Output Size");
            ImGui::InputInt("##CompOutputSize", (int*)&output_size);

            if (input_size != scene->component_context->inputs.size()
                || output_size != scene->component_context->outputs.size()) {
                scene->component_context->setup(input_size, output_size);
                tabs::notify();
            }
            EndSection();
        }
        if (scene != nullptr && !scene->dependencies().empty()) {
            Section("Dependencies");
            if (ImGui::BeginTable("Dependencies", 4,
                    ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
                ImGui::TableHeader("Dependencies");
                ImGui::TableSetupColumn(
                    "Author", ImGuiTableColumnFlags_WidthFixed);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    "Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    "Version", ImGuiTableColumnFlags_WidthFixed);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    "Number of Nodes", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                for (const auto& dep : scene->dependencies()) {
                    const std::string d = dep.to_dependency();
                    bool selected       = false;
                    size_t author_end   = d.find_first_of('/') + 1;
                    size_t name_end     = d.find_last_of('/') + 1;
                    ImGui ::TableNextRow();
                    ImGui ::TableSetColumnIndex(0);
                    ImGui::Selectable(("##" + d).c_str(), &selected,
                        ImGuiSelectableFlags_SpanAllColumns);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(d.substr(0, author_end - 1).c_str());
                    ImGui ::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(
                        d.substr(author_end, name_end - author_end - 1)
                            .c_str());
                    ImGui ::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(d.substr(name_end).c_str());
                    ImGui ::TableSetColumnIndex(3);
                    int count = 0;
                    // FIXME
                    //                  for (auto& x : scene->_components) {
                    //                      if (x.path == d) {
                    //                          count++;
                    //                      }
                    //                  }
                    ImGui::Text("%d", count);
                    if (selected) {
                        Toast(ICON_LC_CLIPBOARD_COPY, "Clipboard",
                            "Dependency name was copied to the clipboard.");
                        ImGui::SetClipboardText(d.c_str());
                    }
                }
                ImGui::EndTable();
                IconButton<NORMAL>(ICON_LC_PACKAGE, "Add Component");
            }
            EndSection();
        }
        if (IconButton<NORMAL>(ICON_LC_UPLOAD, "Upload")) {
            std::string resp;
            // net::upload_scene(&scene, resp);
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

} // namespace lcs::ui::layout
