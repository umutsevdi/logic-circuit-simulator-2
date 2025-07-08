#include "IconsLucide.h"
#include "io.h"
#include "net.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include "ui/popup.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>
#include <string>
#include <string_view>

namespace lcs::ui {

static void tab_window(void);

bool pref_show = false;
bool df_show   = false;
void MenuBar(void)
{
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

    ImGui::PushFont(get_font(FontFlags::NORMAL));
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (IconButton<NORMAL>(ICON_LC_PLUS, "New")) {
                new_flow_show = true;
            }
            if (IconButton<NORMAL>(ICON_LC_FOLDER_OPEN, "Open")) {
                open_flow();
            }
            if (IconButton<NORMAL>(ICON_LC_SAVE, "Save")) {
                if (io::scene::save() == Error::NO_SAVE_PATH_DEFINED) {
                    save_as_flow("Save scene");
                };
            }

            if (IconButton<NORMAL>(ICON_LC_SAVE_ALL, "Save As")) {
                save_as_flow("Save scene as");
            }
            if (IconButton<NORMAL>(ICON_LC_SETTINGS_2, "Preferences")) {
                pref_show = true;
            }
            if (IconButton<NORMAL>(ICON_LC_X, "Close")) {
                close_flow();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::Checkbox("Palette", &user_data.palette);
            ImGui::Checkbox("Inspector", &user_data.inspector);
            ImGui::Checkbox("Console", &user_data.console);
            ImGui::Checkbox("Scene Info", &user_data.scene_info);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
            - ImGui::GetStyle().ItemInnerSpacing.y);

        tab_window();
        ImVec2 pos        = ImGui::GetWindowContentRegionMax();
        ImVec2 login_text = ImGui::CalcTextSize("Login");

        ImGui::PushStyleColor(
            ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
            ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        if (net::is_logged_in()) {
            const ImageHandle* profile
                = get_texture(net::get_account().avatar_url);
            if (profile) {
                ImGui::PushStyleVar(
                    ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.1f, 0.1f));
                float h_image = (1.2 * login_text.y
                    + 2 * ImGui::GetStyle().ItemInnerSpacing.y);
                ImGui::SameLine(pos.x - ImGui::GetStyle().ItemSpacing.x
                    - profile->w / profile->h * h_image);
                if (ImGui::ImageButton(net::get_account().login.c_str(),
                        profile->gl_id,
                        ImVec2(profile->w / profile->h * h_image, h_image),
                        ImVec2(0, 0), ImVec2(1, 1))) {
                    ImGui::OpenPopup("Profile Settings");
                }
                if (ImGui::BeginPopup(
                        "Profile Settings", ImGuiWindowFlags_ChildMenu)) {
                    if (IconButton<NORMAL>(ICON_LC_SETTINGS_2, "Settings")) { }
                    if (IconButton<NORMAL>(ICON_LC_LOG_OUT, "Log out")) {
                        net::get_flow().resolve();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
            }
        } else {
            ImGui::SameLine(pos.x - 2 * login_text.x);
            ImGui::BeginDisabled(
                net::get_flow().get_state() == Flow::State::POLLING);
            ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
                + ImGui::GetStyle().ItemInnerSpacing.y);
            if (IconButton<NORMAL>(ICON_LC_LOG_IN, "Login")) {
                L_INFO("Login button pressed");
                df_show = true;
            }
            ImGui::EndDisabled();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
        LoginWindow(df_show);
        ImGui::Separator();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    Preferences(pref_show);
}

void tab_window(void)
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
        new_flow_show = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}

} // namespace lcs::ui
