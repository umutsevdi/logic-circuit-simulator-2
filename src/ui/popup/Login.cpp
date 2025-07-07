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
    if (net::get_flow().get_state() == Flow::INACTIVE) {
        err = net::get_flow().start();
    }
    if (err) {
        df_show = false;
        net::get_flow().resolve();
        return;
    }
    Flow::State poll_result = net::get_flow().poll();
    if (poll_result == Flow::DONE) {
        df_show = false;
        return;
    }
    ImGui::OpenPopup("Login");
    if (ImGui::BeginPopupModal(
            "Login", &df_show_cancel, ImGuiWindowFlags_NoSavedSettings)) {
        static const float modal_size
            = ImGui::CalcTextSize("Enter the code to your browser").x;
        if (poll_result == Flow::BROKEN) {
            ImGui ::PushFont(get_font(FontFlags ::BOLD | FontFlags ::NORMAL));
            ImGui ::TextColored(get_active_style().red, "An error occurred.");
            ImGui ::PopFont();
            ImGui::Text("%s", net::get_flow().reason());
        } else if (poll_result == Flow::TIMEOUT) {
            ImGui ::PushFont(get_font(FontFlags ::BOLD | FontFlags ::NORMAL));
            ImGui ::TextColored(get_active_style().red,
                "You have exceeded the time limit for entering your\n"
                "passcode. Please try again to authenticate your\n"
                "session. ");
            ImGui ::PopFont();
        } else {
            Field("Enter the code to your browser");
        }
        const char* icon;
        switch (poll_result) {
        case Flow::TIMEOUT: icon = ICON_LC_CLOCK_ALERT; break;
        case Flow::BROKEN: icon = ICON_LC_TRIANGLE_ALERT; break;
        default: icon = ICON_LC_GITHUB; break;
        }
        IconText<ULTRA>(icon, "");

        float icon_h = 0.f;
        {
            ImGui::PushFont(get_font(ULTRA | ICON));
            icon_h = ImGui::CalcTextSize(ICON_LC_GITHUB).y;
            ImGui::PopFont();
        }
        if (poll_result == Flow::POLLING) {
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
                ImGui::SetClipboardText(net::get_flow().user_code.c_str());
            }
        }
        if (poll_result == Flow::State::BROKEN
            || poll_result == Flow::State::TIMEOUT) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + modal_size / 2);
            if (IconButton<NORMAL>(ICON_LC_ROTATE_CW, "Retry")) {
                df_show = true;
                net::get_flow().resolve();
            }
        }
        ImGui::EndPopup();
    }
    if (!df_show_cancel) {
        L_INFO("User pressed cancel");
        df_show = false;
        net::get_flow().resolve();
    }
}
} // namespace lcs::ui
