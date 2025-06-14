#include <string>
#include <string_view>

#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

#include "IconsLucide.h"
#include "io.h"
#include "net.h"
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

net::DeviceFlow df;
bool device_modal_show = false;
bool device_flow       = false;
void MenuBar(void)
{
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));

    ImGui::PushFont(get_font(font_flags_t::NORMAL));
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (IconButton("##New", ICON_LC_PLUS, "New")) {
                extern bool show;
                show = true;
            }
            if (IconButton("##Open", ICON_LC_FOLDER_OPEN, "Open")) {
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
            if (IconButton("##Save", ICON_LC_SAVE, "Save")) {
                if (io::scene::save() == Error::NO_SAVE_PATH_DEFINED) {
                    save_as_flow("Save scene");
                };
            }

            if (IconButton("##SaveAs", ICON_LC_SAVE_ALL, "Save As")) {
                save_as_flow("Save scene as");
            }
            if (IconButton("##Close", ICON_LC_X, "Close")) {
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
        ImGui::Separator();
        if (!net::is_logged_in()) {
            if (IconButton("##LoginButton", ICON_LC_LOG_IN, "Login")) {
                device_flow       = true;
                device_modal_show = true;
            }
        } else {
            if (IconButton(
                    "##LoginButton", ICON_LC_DOOR_CLOSED_LOCKED, "Logout")) { }
        }
        ImGui::EndMainMenuBar();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    if (device_modal_show) {
        if (device_flow) {
            device_flow = false;
            if (net::start_device_flow(df)) {
                device_modal_show = false;
                return;
            };
        }
        ImGui::OpenPopup("Login");
        if (ImGui::BeginPopupModal(
                "Login", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
            Field("Enter the code to your browser");
            ImGui::SameLine();
            ImGui::Text("%s", df.user_code.c_str());
            ImGui::ProgressBar(
                static_cast<float>(difftime(time(nullptr), df.start_time))
                    / static_cast<float>(df.expires_in),
                ImGui::CalcTextSize("LONGLONGTEXT"));
            ImGui::SetClipboardText(df.user_code.c_str());
            switch (df.poll()) {
            case net::DeviceFlow::IDLE:
            case net::DeviceFlow::BROKEN:
            case net::DeviceFlow::POLLING: break;
            case net::DeviceFlow::DONE:
            case net::DeviceFlow::TIMEOUT: device_modal_show = false; break;
            }
            ImGui::EndPopup();
        }
    }
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
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    ImGui::PushFont(get_font(ICON | SMALL));
    if (ImGui::TabItemButton(ICON_LC_PLUS)) {
        extern bool show;
        show = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}
} // namespace lcs::ui
