#include "IconsLucide.h"
#include "net.h"
#include "ui/components.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

namespace lcs::ui {

void LoginWindow(bool& df_show)
{
    if (!df_show) {
        return;
    }
    bool df_show_cancel = true;
    Error err           = Error::OK;
    if (net::get_flow().get_state() == net::AuthenticationFlow::INACTIVE) {
        err = net::get_flow().start();
    }
    if (err) {
        Toast(ICON_LC_WIFI_OFF, "Login Error",
            "Failed to connect to the server.", true);
        df_show = false;
        net::get_flow().resolve();
        return;
    }
    net::AuthenticationFlow::State poll_result = net::get_flow().poll();
    switch (poll_result) {
    case net::AuthenticationFlow::DONE:
        df_show = false;
        Toast(ICON_LC_GITHUB, ("Welcome " + net::get_account().name).c_str(),
            "Authentication was successful.");
        return;
    case net::AuthenticationFlow::TIMEOUT:
        Toast(ICON_LC_CLOCK_ALERT, "Login Timeout",
            "You have exceeded the time limit for\n"
            "entering your passcode. Please try again\n"
            "to authenticate your session.",
            true);
        df_show_cancel = false;
        break;
    case net::AuthenticationFlow::BROKEN:
        Toast(ICON_LC_TRIANGLE_ALERT, "Login Error", net::get_flow().reason(),
            true);
        df_show_cancel = false;
        break;
    default: break;
    }
    if (df_show_cancel) {
        ImGui::OpenPopup("Login");
    }
    if (ImGui::BeginPopupModal(
            "Login", &df_show_cancel, ImGuiWindowFlags_NoSavedSettings)) {
        static const float modal_size
            = ImGui::CalcTextSize("Enter the code to your browser").x;
        Field("Enter the code to your browser");
        IconText<ULTRA>(ICON_LC_GITHUB, "");
        float icon_h = 0.f;
        ImGui::PushFont(get_font(ULTRA | ICON));
        icon_h = ImGui::CalcTextSize(ICON_LC_GITHUB).y;
        ImGui::PopFont();
        if (poll_result == net::AuthenticationFlow::POLLING) {
            ImGui::SameLine();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + icon_h / 2
                - ImGui::CalcTextSize("Enter").y);
            ImGui::PushFont(get_font(FontFlags::LARGE | FontFlags::BOLD));
            ImGui::Text("%s", net::get_flow().user_code.c_str());
            ImGui::PopFont();
            ImGui::SetNextItemWidth(modal_size);
            ImGui::ProgressBar(1.0f
                    - static_cast<float>(
                          difftime(time(nullptr), net::get_flow().start_time))
                        / static_cast<float>(net::get_flow().expires_in),
                ImVec2 { modal_size, ImGui::CalcTextSize("Remaining").y },
                ("Remaining: "
                    + std::to_string(net::get_flow().expires_in
                        - static_cast<int>(difftime(
                            time(nullptr), net::get_flow().start_time)))
                    + " seconds")
                    .c_str());
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + modal_size / 2);
            if (IconButton<LARGE>(ICON_LC_CLIPBOARD_COPY, "Copy")) {
                Toast(ICON_LC_CLIPBOARD_COPY, "Clipboard",
                    "Token was copied to the clipboard.");
                ImGui::SetClipboardText(net::get_flow().user_code.c_str());
            }
        }
        ImGui::EndPopup();
    }
    if (!df_show_cancel) {
        L_INFO("Operation was cancelled.");
        df_show = false;
        net::get_flow().resolve();
    }
}
} // namespace lcs::ui
