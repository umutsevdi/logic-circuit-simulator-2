#include "IconsLucide.h"
#include "imgui.h"
#include "io.h"
#include "net.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/layout.h"

namespace lcs::ui {
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
            io::scene::notify_change();
        };

        Field("Description");
        if (scene != nullptr
            && ImGui::InputTextMultiline("##SceneDescInputText",
                scene->description.data(), scene->description.max_size(),
                ImVec2(ImGui::GetWindowWidth(),
                    ImGui::CalcTextSize("\n\n\n").y))) {
            io::scene::notify_change();
        };

        Field("Version");
        if (scene != nullptr
            && ImGui::InputInt("##SceneVersion", &scene->version)) {
            scene->version = std::max(scene->version, 0);
            io::scene::notify_change();
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
                io::scene::notify_change();
            }
            EndSection();
        }
        if (IconButton<NORMAL>(ICON_LC_UPLOAD, "Upload")) {
            std::string resp;
            net::upload_scene(&scene, resp);
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

} // namespace lcs::ui
