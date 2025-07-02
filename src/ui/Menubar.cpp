#include "IconsLucide.h"
#include "io.h"
#include "net.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>
#include <string>
#include <string_view>

namespace lcs::ui {

static void _show_device_flow_ui();
static void _show_preferences_ui();

Configuration cfg;
bool pref_show = false;
bool df_show   = false;
void MenuBar(void)
{
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));

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
                cfg       = get_config();
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
        ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
            - ImGui::GetStyle().ItemInnerSpacing.y);

        TabWindow();
        ImVec2 pos = ImGui::GetWindowContentRegionMax();

        ImGui::PushStyleColor(
            ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        if (net::is_logged_in()) {

            ImVec2 login_text
                = ImGui::CalcTextSize(net::get_account().login.c_str());
            ImGui::SameLine(pos.x - 2 * login_text.x);
            ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
                - ImGui::GetStyle().ItemInnerSpacing.y);
            if (ImGui::BeginMenu(net::get_account().login.c_str())) {
                if (IconButton<NORMAL>(ICON_LC_SETTINGS_2, "Settings")) { }
                if (IconButton<NORMAL>(ICON_LC_LOG_OUT, "Log out")) {
                    net::get_flow().resolve();
                }
                ImGui::EndMenu();
            };
        } else {
            ImVec2 login_text = ImGui::CalcTextSize("Login");
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
        ImGui::EndMainMenuBar();
        if (df_show) {
            _show_device_flow_ui();
        }
        ImGui::Separator();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    _show_preferences_ui();

    ImGui::SameLine();
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
}

static void _show_device_flow_ui()
{
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

void _show_preferences_ui()
{
    if (!pref_show) {
        return;
    }
    const static ImVec2 __table_l_size
        = ImGui::CalcTextSize("LIGHT THEME STYLE");
    ImGui::Begin("Preferences", &pref_show,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | (!cfg.is_saved ? ImGuiWindowFlags_UnsavedDocument
                             : ImGuiWindowFlags_None));

    ImGui::SetWindowPos(
        ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing);
    Section("Appearance");
    if (ImGui::BeginTable(
            "##AppearanceTable", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn(
            "##Key", ImGuiTableColumnFlags_WidthFixed, __table_l_size.x);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

        TablePair(Field("Font Size"));
        if (ImGui::InputFloat("##FontSize", &cfg.font_size, 0.5, 1.0, "%.1f")) {
            if (cfg.font_size < 4) {
                cfg.font_size = 4;
            }
            cfg.is_applied = false;
        };
        TablePair(Field("UI Scale"));
        if (ImGui::InputInt("##UIScale", &cfg.scale, 5, 10)) {
            if (cfg.scale < 5) {
                cfg.scale = 5;
            }
            cfg.is_applied = false;
        };
        {
            TablePair(Field("Light Theme Style"));
            int selected = cfg.light_theme;
            static const char* table[]
                = { "Seoul256 Light", "Acme", "Gruvbox Light", "One Light" };
            if (ImGui::Combo("##Select Theme", &selected, table, 4)) {
                cfg.is_applied  = false;
                cfg.light_theme = (Style)selected;
            }
            const LcsStyle& style = ui::get_style(cfg.light_theme);
            ImGui::ColorButton("Background (Light)", style.bg);
            ImGui::SameLine();
            ImGui::ColorButton("Black (Light)", style.black);
            ImGui::SameLine();
            ImGui::ColorButton("Red (Light)", style.red);
            ImGui::SameLine();
            ImGui::ColorButton("Green (Light)", style.green);
            ImGui::SameLine();
            ImGui::ColorButton("Yellow (Light)", style.yellow);
            ImGui::SameLine();
            ImGui::ColorButton("Blue (Light)", style.blue);
            ImGui::SameLine();
            ImGui::ColorButton("Magenta (Light)", style.magenta);
            ImGui::SameLine();
            ImGui::ColorButton("Cyan (Light)", style.cyan);
            ImGui::SameLine();
            ImGui::ColorButton("White (Light)", style.white);

            ImGui::ColorButton("Foreground (Light)", style.fg);
            ImGui::SameLine();
            ImGui::ColorButton("Black Bright (Light)", style.black_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Red Bright (Light)", style.red_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Green Bright (Light)", style.green_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Yellow Bright (Light)", style.yellow_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Blue Bright (Light)", style.blue_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Magenta Bright (Light)", style.magenta_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Cyan Bright (Light)", style.cyan_bright);
            ImGui::SameLine();
            ImGui::ColorButton("White Bright (Light)", style.white_bright);
        }
        {
            TablePair(Field("Dark Theme Style"));

            int selected = cfg.dark_theme - 4;
            static const char* table[]
                = { "Seashells", "XTerm", "Gruvbox Dark", "One Dark" };
            if (ImGui::Combo("##Select Theme Dark", &selected, table, 4)) {
                cfg.dark_theme = (Style)(selected + 4);
                cfg.is_applied = false;
            }
            const LcsStyle& style = ui::get_style(cfg.dark_theme);
            ImGui::ColorButton("Background (Dark)", style.bg);
            ImGui::SameLine();
            ImGui::ColorButton("Black (Dark)", style.black);
            ImGui::SameLine();
            ImGui::ColorButton("Red (Dark)", style.red);
            ImGui::SameLine();
            ImGui::ColorButton("Green (Dark)", style.green);
            ImGui::SameLine();
            ImGui::ColorButton("Yellow (Dark)", style.yellow);
            ImGui::SameLine();
            ImGui::ColorButton("Blue (Dark)", style.blue);
            ImGui::SameLine();
            ImGui::ColorButton("Magenta (Dark)", style.magenta);
            ImGui::SameLine();
            ImGui::ColorButton("Cyan (Dark)", style.cyan);
            ImGui::SameLine();
            ImGui::ColorButton("White (Dark)", style.white);

            ImGui::ColorButton("Foreground (Dark)", style.fg);
            ImGui::SameLine();
            ImGui::ColorButton("Black Bright (Dark)", style.black_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Red Bright (Dark)", style.red_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Green Bright (Dark)", style.green_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Yellow Bright (Dark)", style.yellow_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Blue Bright (Dark)", style.blue_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Magenta Bright (Dark)", style.magenta_bright);
            ImGui::SameLine();
            ImGui::ColorButton("Cyan Bright (Dark)", style.cyan_bright);
            ImGui::SameLine();
            ImGui::ColorButton("White Bright (Dark)", style.white_bright);
        }
        static const char* pref_table[] = {
            "Follow OS",
            "Always Light",
            "Always Dark",
        };
        TablePair(Field("Rounded Corners"));
        if (ImGui::Checkbox("##Rounded Corners", &cfg.rounded_corners)) {
            cfg.is_applied = false;
        };
        TablePair(Field("Theme Preference"));
        if (ImGui::Combo(
                "##ThemePreference", (int*)&cfg.preference, pref_table, 3)) {
            cfg.is_applied = false;
        }
        TablePair(Field("Startup Window"));
        Point p = { cfg.startup_win_x, cfg.startup_win_y };
        if (PositionSelector(p, "##StartupWindow")) {
            cfg.startup_win_x = p.x;
            cfg.startup_win_y = p.y;
            cfg.is_applied    = false;
        }
        ImGui::EndTable();
    }
    EndSection();
    Section("Network");
    if (ImGui::BeginTable("##NetworkTable", 2, ImGuiTableFlags_BordersInnerV)) {
        static std::array<char, 1024> api_input {};
        ImGui::TableSetupColumn(
            "##Key", ImGuiTableColumnFlags_WidthFixed, __table_l_size.x);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);
        TablePair(Field("API Proxy"));
        if (ImGui::InputText(
                "##api_proxy", api_input.data(), api_input.max_size())) {
            cfg.is_applied = false;
            cfg.api_proxy  = api_input.begin();
        }
        ImGui::EndTable();
    }
    EndSection();
    ImGui::BeginDisabled(cfg.is_applied);
    if (IconButton<NORMAL>(ICON_LC_REDO_DOT, "Apply")) {
        set_config(cfg);
        cfg            = get_config();
        cfg.is_applied = true;
    }
    ImGui::EndDisabled();
    ImGui::SameLine(ImGui::GetWindowSize().x * 5 / 6);
    ImGui::BeginDisabled(cfg.is_saved);
    if (IconButton<NORMAL>(ICON_LC_SAVE, "Save")) {
        set_config(cfg);
        cfg = get_config();
        save_config();
        cfg.is_applied = true;
        cfg.is_saved   = true;
    }
    ImGui::EndDisabled();
    ImGui::End();
}

} // namespace lcs::ui
