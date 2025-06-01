#include <imgui.h>
#include <imnodes.h>
#include <string>
#include <tinyfiledialogs.h>

#include "io.h"
#include "ui.h"
#include "ui/layout.h"

namespace lcs::ui {
static const char* _PATH_FILTER[1] = { "*.json" };

static inline bool _save_as_flow(const char* title)
{
    const char* new_path = tinyfd_saveFileDialog(
        title, io::LOCAL.c_str(), 1, _PATH_FILTER, "Save the scene as");
    if (new_path != nullptr) {
        std::string path_as_str { new_path };
        if (path_as_str.find(".json") != std::string::npos) {
            return io::scene::save_as(new_path) == error_t::OK;
        } else {
            return io::scene::save_as(path_as_str + ".json") == error_t::OK;
        }
    }
    return false;
}

static void _close_flow()
{
    if (io::scene::get() == nullptr) { exit(0); }
    if (io::scene::is_saved()) {
        io::scene::close();
    } else {
        int option = tinyfd_messageBox("Close Scene",
            "You have unsaved changes. Would you like to save your changes "
            "before closing?",
            "yesno", "question", 0);
        if (!option || _save_as_flow("Save scene")) { io::scene::close(); }
    }
}

void MenuBar(void)
{
    ImGui::PushFont(get_font(font_flags_t::NORMAL));
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                io::scene::get(io::scene::create("Untitled Scene"));
            }
            if (ImGui::MenuItem("Open")) {
                const char* path = tinyfd_openFileDialog("Select a scene",
                    io::LIBRARY.c_str(), 1, _PATH_FILTER, "LCS Scene File", 0);
                if (path != nullptr) {
                    size_t idx;
                    error_t err = io::scene::open(path, idx);
                    if (err) { ERROR(err); }
                }
            }
            if (ImGui::MenuItem("Save")) {
                if (io::scene::save() == error_t::NO_SAVE_PATH_DEFINED) {
                    _save_as_flow("Save scene");
                };
            }

            if (ImGui::MenuItem("Save As")) { _save_as_flow("Save scene as"); }
            if (ImGui::MenuItem("Close")) { _close_flow(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("View")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }
        ImGui::EndMainMenuBar();
    };
    ImGui::PopFont();
}

bool TabWindow(void)
{
    ImGui::BeginTabBar("Scene Tabs", ImGuiTabBarFlags_FittingPolicyScroll);
    bool scene_changed = false;
    io::scene::iterate([&scene_changed](size_t idx, const std::string& path,
                           bool is_saved, bool is_active) -> bool {
        bool result            = false;
        bool keep              = true;
        std::string scene_name = path != ""
            ? path
            : ("Untitled Scene (" + std::to_string(idx) + ")");

        if (ImGui::BeginTabItem(scene_name.c_str(), &keep,
                (is_saved ? ImGuiTabItemFlags_None
                          : ImGuiTabItemFlags_UnsavedDocument)
                    | (is_active & 2 ? ImGuiTabItemFlags_SetSelected
                                     : ImGuiTabItemFlags_None))) {
            if (!is_active) {
                L_INFO("Change scene event " << idx);
                result        = true;
                scene_changed = true;
            }
            ImGui::EndTabItem();
        }
        if (!keep) { _close_flow(); }
        return result;
    });
    ImGui::PushStyleColor(
        ImGuiCol_Tab, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    if (ImGui::TabItemButton("+")) {
        io::scene::get(io::scene::create("Untitled Scene"));
    }
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
    return scene_changed;
}
} // namespace lcs::ui
