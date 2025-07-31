
#include "IconsLucide.h"
#include "components.h"
#include "port.h"
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <string>

namespace lcs::ui {

static const char* _PATH_FILTER[1] = { "*.json" };

void open_flow()
{
    const char* path = tinyfd_openFileDialog("Select a scene", LIBRARY.c_str(),
        1, _PATH_FILTER, "LCS Scene File", 0);
    if (path != nullptr) {
        size_t idx;
        Error err = io::scene::open(path, idx);
        if (err) {
            ERROR(err);
        }
    }
}
bool save_as_flow(const char* title)
{

    const char* new_path = tinyfd_saveFileDialog(
        title, LOCAL.c_str(), 1, _PATH_FILTER, "Save the scene as");
    if (new_path != nullptr) {
        std::string path_as_str { new_path };
        if (path_as_str.find(".json") != std::string::npos) {
            return io::scene::save_as(new_path) == Error::OK;
        } else {
            return io::scene::save_as(path_as_str + ".json") == Error::OK;
        }
    }
    return false;
}

void close_flow(void)
{
    if (io::scene::get() == nullptr) {
        exit(0);
    }
    if (io::scene::is_saved()) {
        io::scene::close();
    } else {
        int option = tinyfd_messageBox("Close Scene",
            "You have unsaved changes. Would you like to save your changes "
            "before closing?",
            "yesno", "question", 0);
        if (!option || save_as_flow("Save scene")) {
            io::scene::close();
        }
    }
}

bool new_flow_show;
void new_flow(void)
{
    if (!new_flow_show) {
        return;
    }
    ImGui::OpenPopup("New Scene");
    if (ImGui::BeginPopupModal("New Scene", &new_flow_show,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        static bool is_scene         = true;
        static char author[60]       = "local";
        static char name[128]        = { 0 };
        static char description[512] = { 0 };
        static size_t input_size     = 0;
        static size_t output_size    = 0;

        if (ImGui::BeginTable("##NewFlow", 2, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn(
                "##NewFlowKey", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "##Value", ImGuiTableColumnFlags_WidthStretch);

            TablePair(Field("Scene Name"),
                ImGui::InputText("##SceneCreate_Name", name, 128,
                    ImGuiInputTextFlags_CharsNoBlank));

            TablePair(Field("Author"),
                ImGui::PushFont(
                    get_font(FontFlags::ITALIC | FontFlags::NORMAL)),
                ImGui::InputText("##SceneCreate_Author", author, 60,
                    ImGuiInputTextFlags_ReadOnly),
                ImGui::PopFont());

            TablePair(Field("Description"),
                ImGui::InputTextMultiline(
                    "##SceneCreate_Desc", description, 512));

            TablePair(Field("Type"));
            if (ImGui::RadioButton("##IsScene", is_scene)) {
                is_scene = true;
            }
            ImGui::SameLine();
            IconText<NORMAL>(ICON_LC_LAND_PLOT, "Scene");
            ImGui::SameLine();
            if (ImGui::RadioButton("##IsComponent", !is_scene)) {
                is_scene = false;
            }
            ImGui::SameLine();
            IconText<NORMAL>(ICON_LC_PACKAGE, "Component");

            if (!is_scene) {
                ImGui::Separator();
                TablePair(Field("Input Size"));
                ImGui::InputInt("##CompInputSize", (int*)&input_size);
                TablePair(Field("Output Size"));
                ImGui::InputInt("##CompOutputSize", (int*)&output_size);
            }
            TablePair(
                if (ImGui::Button("Create")) {
                    NRef<Scene> scene = io::scene::get(
                        io::scene::create(name, author, description));
                    if (!is_scene) {
                        scene->component_context.emplace(
                            &scene, input_size, output_size);
                    }
                    new_flow_show = false;
                },
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    new_flow_show = false;
                });
            ImGui::EndTable();
        };
        ImGui::EndPopup();
    }
}

} // namespace lcs::ui
