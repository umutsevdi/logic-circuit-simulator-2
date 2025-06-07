#include <string>
#include <string_view>

#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

#include "io.h"
#include "ui/layout.h"
#include "ui/util.h"

namespace lcs::ui {
static const char* _PATH_FILTER[1] = { "*.json" };

bool save_as_flow(const char* title)
{
    const char* new_path = tinyfd_saveFileDialog(
        title, io::LOCAL.c_str(), 1, _PATH_FILTER, "Save the scene as");
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

void MenuBar(void)
{
    ImGui::PushFont(get_font(font_flags_t::NORMAL));
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                extern bool show;
                show = true;
            }
            if (ImGui::MenuItem("Open")) {
                const char* path = tinyfd_openFileDialog("Select a scene",
                    io::LIBRARY.c_str(), 1, _PATH_FILTER, "LCS Scene File", 0);
                if (path != nullptr) {
                    size_t idx;
                    Error err = io::scene::open(path, idx);
                    if (err) {
                        ERROR(err);
                    }
                }
            }
            if (ImGui::MenuItem("Save")) {
                if (io::scene::save() == Error::NO_SAVE_PATH_DEFINED) {
                    save_as_flow("Save scene");
                };
            }

            if (ImGui::MenuItem("Save As")) {
                save_as_flow("Save scene as");
            }
            if (ImGui::MenuItem("Close")) {
                close_flow();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    };
    ImGui::PopFont();
}

void TabWindow(void)
{
    ImGui::BeginTabBar("Scene Tabs", ImGuiTabBarFlags_FittingPolicyScroll);
    io::scene::iterate([](std::string_view name, std::string_view path,
                           bool is_saved, bool is_active) -> bool {
        bool result = false;
        bool keep   = true;

        std::string_view scene_name;
        if (!name.empty()) {
            scene_name = name;
        } else if (!path.empty()) {
            scene_name = path;
        } else {
            scene_name = "Untitled Scene";
        }
        if (ImGui::BeginTabItem(scene_name.begin(), &keep,
                (is_saved ? ImGuiTabItemFlags_None
                          : ImGuiTabItemFlags_UnsavedDocument))) {
            if (!is_active) {
                result = true;
                ImNodes::ClearLinkSelection();
                ImNodes::ClearNodeSelection();
            }
            ImGui::EndTabItem();
        }
        if (!keep) {
            close_flow();
        }
        return result;
    });
    ImGui::PushStyleColor(
        ImGuiCol_Tab, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    if (ImGui::TabItemButton("+")) {
        extern bool show;
        show = true;
    }
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}
} // namespace lcs::ui
