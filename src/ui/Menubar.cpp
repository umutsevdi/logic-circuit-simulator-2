#include "IconsLucide.h"
#include "io.h"
#include "net.h"
#include "ui/components.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>
#include <string>
#include <string_view>

namespace lcs::ui {

static void _show_device_flow_ui();

bool df_show = false;
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
            if (IconButton<NORMAL>(ICON_LC_X, "Close")) {
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

        ImGui::EndMainMenuBar();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
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
        new_flow_show = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndTabBar();

    ImGui::SameLine();
    if (net::is_logged_in()) {
        // TODO Show image
        if (ImGui::BeginMenu(net::get_account().login.c_str())) {
            if (IconButton<NORMAL>(
                    "##Settings", ICON_LC_SETTINGS_2, "Settings")) { }
            if (IconButton<NORMAL>(ICON_LC_LOG_OUT, "Log out")) {
                net::get_flow().resolve();
            }
            ImGui::EndMenu();
        };
    } else {
        ImGui::BeginDisabled(
            net::get_flow().get_state() == Flow::State::POLLING);
        if (IconButton<NORMAL>(ICON_LC_LOG_IN, "Login")) {
            df_show = true;
        }
        ImGui::EndDisabled();
    }
    if (df_show) {
        _show_device_flow_ui();
    }
}

static void _show_device_flow_ui()
{
    Error err = Error::OK;
    if (net::get_flow().get_state() == Flow::INACTIVE) {
        net::get_flow().start();
    }
    if (err) {
        df_show = false;
        net::get_flow().resolve();
        return;
    }
    Flow::State poll_result = net::get_flow().poll();
    ImGui::OpenPopup("Login");
    if (ImGui::BeginPopupModal(
            "Login", nullptr, ImGuiWindowFlags_NoSavedSettings)) {

        const char* icon;
        switch (poll_result) {
        case Flow::TIMEOUT: icon = ICON_LC_CLOCK_ALERT; break;
        case Flow::BROKEN: icon = ICON_LC_TRIANGLE_ALERT; break;
        default: icon = ICON_LC_GITHUB; break;
        }
        IconText<ULTRA>(icon, "");
        ImGui::SameLine();
        if (poll_result == Flow::BROKEN) {
            ImGui ::PushFont(
                get_font(font_flags_t ::BOLD | font_flags_t ::NORMAL));
            ImGui ::TextColored(ImVec4(255, 0, 0, 255), "An error occurred.");
            ImGui ::PopFont();
            ImGui::Text("%s", net::get_flow().reason());
        } else if (poll_result == Flow::TIMEOUT) {
            ImGui ::PushFont(
                get_font(font_flags_t ::BOLD | font_flags_t ::NORMAL));
            ImGui ::TextColored(ImVec4(255, 55, 55, 255),
                "You have exceeded the time limit for entering your\n"
                "passcode. Please try again to authenticate your\n"
                "session. ");
            ImGui ::PopFont();
        } else {
            Field("Enter the code to your browser");
        }
        if (poll_result == Flow::POLLING) {
            ImGui::PushFont(get_font(font_flags_t::LARGE | font_flags_t::BOLD));
            ImGui::Text("%s", net::get_flow().user_code.c_str());
            ImGui::PopFont();
            ImGui::SameLine();
            if (IconButton<LARGE>(
                    "##CopyToClipboard", ICON_LC_CLIPBOARD_COPY, "")) {
                ImGui::SetClipboardText(net::get_flow().user_code.c_str());
            }
            ImGui::ProgressBar(1.0f
                    - static_cast<float>(
                          difftime(time(nullptr), net::get_flow().start_time))
                        / static_cast<float>(net::get_flow().expires_in),
                ImVec2 { ImGui::GetWindowSize().x,
                    ImGui::CalcTextSize("Remaining").y },
                ("Remaining: "
                    + std::to_string(net::get_flow().expires_in
                        - static_cast<int>(difftime(
                            time(nullptr), net::get_flow().start_time)))
                    + " seconds")
                    .c_str());
        }
        if (poll_result == Flow::State::BROKEN
            || poll_result == Flow::State::TIMEOUT) {
            if (IconButton<NORMAL>(ICON_LC_ROTATE_CW, "Retry")) {
                df_show = true;
                net::get_flow().resolve();
            }
            ImGui::SameLine(ImGui::GetWindowSize().x / 2);
            if (IconButton<NORMAL>(ICON_LC_CIRCLE_X, "Cancel")) {
                df_show = false;
                net::get_flow().resolve();
            }
        }
        ImGui::EndPopup();
    }
}
} // namespace lcs::ui
