#include "IconsLucide.h"
#include "common.h"
#include "components.h"
#include "configuration.h"
#include "port.h"
#include "ui.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui::layout {

Configuration cfg {};
static bool _show_new   = false;
static bool _show_close = false;
static bool _show_about = false;
static bool _show_pref  = false;
static void tab_window(void);
static void _popup_new(void);
static void _popup_about(void);
static void _popup_close(void);
static void _popup_pref(void);
bool df_show = false;
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
                _show_new = true;
            }
            if (IconButton<NORMAL>(ICON_LC_FOLDER_OPEN, "Open")) {
                dialog::open_file();
            }
            ImGui::BeginDisabled(tabs::active() == nullptr);
            if (IconButton<NORMAL>(ICON_LC_SAVE, "Save")) {
                if (tabs::save() == Error::NO_SAVE_PATH_DEFINED) {
                    dialog::save_file_as();
                };
            }

            if (IconButton<NORMAL>(ICON_LC_SAVE_ALL, "Save As")) {
                dialog::save_file_as();
            }
            ImGui::EndDisabled();
            if (IconButton<NORMAL>(ICON_LC_SETTINGS_2, "Preferences")) {
                _show_pref = true;
            }
            if (IconButton<NORMAL>(ICON_LC_X, "Close")) {
                _popup_close();
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
            if (IconButton<NORMAL>(ICON_LC_INFO, "About")) {
                _show_about = true;
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
            - ImGui::GetStyle().ItemInnerSpacing.y);

        tab_window();
        //        ImVec2 pos        = ImGui::GetWindowContentRegionMax();
        //      ImVec2 login_text = ImGui::CalcTextSize("Login");

        //      ImGui::PushStyleColor(
        //          ImGuiCol_Button,
        //          ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        //      if (auth::is_logged_in()) {
        //          const ImageHandle* profile
        //              = get_texture(net::get_account().avatar_url);
        //          if (profile) {
        //              ImGui::PushStyleVar(
        //                  ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.1f, 0.1f));
        //              float h_image = (1.2 * login_text.y
        //                  + 2 * ImGui::GetStyle().ItemInnerSpacing.y);
        //              ImGui::SameLine(pos.x - ImGui::GetStyle().ItemSpacing.x
        //                  - profile->w / profile->h * h_image);
        //              if (ImGui::ImageButton(net::get_account().login.c_str(),
        //                      profile->gl_id,
        //                      ImVec2(profile->w / profile->h * h_image,
        //                      h_image), ImVec2(0, 0), ImVec2(1, 1))) {
        //                  ImGui::OpenPopup("Profile Settings");
        //              }
        //              if (ImGui::BeginPopup(
        //                      "Profile Settings", ImGuiWindowFlags_ChildMenu))
        //                      {
        //                  if (IconButton<NORMAL>(ICON_LC_SETTINGS_2,
        //                  "Settings")) { } if
        //                  (IconButton<NORMAL>(ICON_LC_LOG_OUT, "Log out")) {
        //                      net::get_flow().resolve();
        //                  }
        //                  ImGui::EndPopup();
        //              }
        //              ImGui::PopStyleVar();
        //          }
        //      } else {
        //          ImGui::SameLine(pos.x - 2 * login_text.x);
        //          ImGui::BeginDisabled(net::get_flow().get_state()
        //              == net::AuthenticationFlow::State::POLLING);
        //          ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
        //              + ImGui::GetStyle().ItemInnerSpacing.y);
        //          if (IconButton<NORMAL>(ICON_LC_LOG_IN, "Login")) {
        //              L_INFO("Login button pressed");
        //              df_show = true;
        //          }
        //          ImGui::EndDisabled();
        //      }
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
        //        popup::LoginWindow(df_show);
        //  ImGui::Separator();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    _popup_new();
    _popup_close();
    _popup_about();
    _popup_pref();
}

void tab_window(void)
{
    ImGui::BeginTabBar("Scene Tabs", ImGuiTabBarFlags_FittingPolicyScroll);
    tabs::for_each([](std::string_view name, std::string_view path,
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
            _popup_close();
        }
        return result;
    });
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    if (ImGui::TabItemButton("+")) {
        _show_new = true;
    }
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}

static void _popup_new(void)
{
    if (!_show_new) {
        return;
    }
    ImGui::OpenPopup("New Scene");
    if (ImGui::BeginPopupModal("New Scene", &_show_new,
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
                    NRef<Scene> scene = tabs::active(
                        tabs::create(name, author, description, 1));
                    if (!is_scene) {
                        scene->component_context.emplace(
                            &scene, input_size, output_size);
                    }
                    _show_new = false;
                },
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    _show_new = false;
                });
            ImGui::EndTable();
        };
        ImGui::EndPopup();
    }
}

static void _popup_close(void)
{
    if (!_show_close) {
        return;
    }
    if (tabs::active() == nullptr) {
        exit(0);
    }
    if (tabs::is_saved()) {
        if (!tabs::close()) {
            _show_close = false;
        }
    } else {
        ImGui::OpenPopup("Close Scene");
        if (ImGui::BeginPopupModal("Close Scene", &_show_close,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text(
                "You have unsaved changes. Would you like to save your changes "
                "before closing?");
            if (IconButton<NORMAL>(ICON_LC_SAVE, "Save & Close")
                && dialog::save_file_as() == Error::OK) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
            ImGui::SameLine();
            if (IconButton<NORMAL>(ICON_LC_SAVE_OFF, "Exit Without Saving")) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
        }
    }
}

static void _popup_about(void)
{
    static const char* site = "https://umutsevdi.com";
    static const char* gh   = "https://github.com/umutsevdi";
    static const char* prj
        = "https://github.com/umutsevdi/logic-circuit-simulator-2";
    static const char* mail
        = "mailto:ask@umutsevdi.com?subject=\'About Logic Circuit Simulator\'";
    if (!_show_about) {
        return;
    }
    ImGui::OpenPopup("About");
    if (ImGui::BeginPopupModal("About", &_show_about,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Logic Circuit Simulator is a free and open-source "
                    "cross-platform desktop\r\n"
                    "application to simulate logic circuits.");
        ImGui::Text("Developed by");
        ImGui::SameLine();
        if (ImGui::TextLink("Umut Sevdi")) {
            open_browser(site);
        }
        ImGui::SameLine();
        ImGui::Text(". Source code is available at ");
        ImGui::SameLine();
        if (ImGui::TextLink("GitHub.")) {
            open_browser(prj);
        }
        ImGui::BeginChild(
            "License", ImVec2(0, 450), ImGuiChildFlags_FrameStyle);
        ImGui::TextUnformatted(__LICENSE__);
        ImGui::EndChild();
        ImGui::TextUnformatted("Contact");
        ImGui::SameLine();
        if (IconButton<NORMAL>(ICON_LC_MAIL, "Email")) {
            open_browser(mail);
        }
        ImGui::SameLine();
        if (IconButton<NORMAL>(ICON_LC_LINK, "Website")) {
            open_browser(site);
        }
        ImGui::SameLine();
        if (IconButton<NORMAL>(ICON_LC_GITHUB, "GitHub")) {
            open_browser(gh);
        }
        ImGui::EndPopup();
    }
}

static void _color_buttons(const LcsTheme& style)
{
    ImGui::PushID(style.name.c_str());
    ImGui::ColorButton("Background", style.bg);
    ImGui::SameLine();
    ImGui::ColorButton("Foreground", style.fg);

    ImGui::ColorButton("Black", style.black);
    ImGui::SameLine();
    ImGui::ColorButton("Red", style.red);
    ImGui::SameLine();
    ImGui::ColorButton("Green", style.green);
    ImGui::SameLine();
    ImGui::ColorButton("Yellow", style.yellow);

    ImGui::ColorButton("Black Bright", style.black_bright);
    ImGui::SameLine();
    ImGui::ColorButton("Red Bright", style.red_bright);
    ImGui::SameLine();
    ImGui::ColorButton("Green Bright", style.green_bright);
    ImGui::SameLine();
    ImGui::ColorButton("Yellow Bright", style.yellow_bright);

    ImGui::ColorButton("Blue", style.blue);
    ImGui::SameLine();
    ImGui::ColorButton("Magenta", style.magenta);
    ImGui::SameLine();
    ImGui::ColorButton("Cyan", style.cyan);
    ImGui::SameLine();
    ImGui::ColorButton("White", style.white);

    ImGui::ColorButton("Blue Bright", style.blue_bright);
    ImGui::SameLine();
    ImGui::ColorButton("Magenta Bright", style.magenta_bright);
    ImGui::SameLine();
    ImGui::ColorButton("Cyan Bright", style.cyan_bright);
    ImGui::SameLine();
    ImGui::ColorButton("White Bright", style.white_bright);
    ImGui::PopID();
}
void _popup_pref(void)
{
    static int light_idx = 0;
    static int dark_idx  = 0;
    static const ImVec2 __table_l_size
        = ImGui::CalcTextSize("LIGHT THEME STYLE");
    static const char* pref_table[] = {
        "Follow OS",
        "Always Light",
        "Always Dark",
    };
    if (!_show_pref) {
        return;
    }
    auto light_themes = get_available_styles(false);
    auto dark_themes  = get_available_styles(true);
    if (cfg.is_saved && cfg.is_applied) {
        cfg = get_config();
        for (size_t i = 0; i < light_themes.size(); i++) {
            if (cfg.light_theme == light_themes[i]) {
                light_idx = i;
            }
        }
        for (size_t i = 0; i < dark_themes.size(); i++) {
            if (cfg.dark_theme == dark_themes[i]) {
                dark_idx = i;
            }
        }
    }
    bool keep = true;
    ImGui::Begin("Preferences", &keep,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoDocking
            | (!get_config().is_saved ? ImGuiWindowFlags_UnsavedDocument
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

        TablePair(Field("UI Scale"));
        if (ImGui::SliderInt("%", &cfg.scale, 75, 150)) {
            cfg.is_applied = false;
        };
        {
            TablePair(Field("Light Theme Style"));
            auto light_themes = get_available_styles(false);
            if (ImGui::Combo("##Select Theme", &light_idx, light_themes.data(),
                    light_themes.size())) {
                cfg.is_applied  = false;
                cfg.light_theme = light_themes[light_idx];
            }
            const LcsTheme& style = ui::get_theme(cfg.light_theme);
            _color_buttons(style);
        }
        {
            TablePair(Field("Dark Theme Style"));
            if (ImGui::Combo("##Select Theme Dark", &dark_idx,
                    dark_themes.data(), dark_themes.size())) {
                cfg.is_applied = false;
                cfg.dark_theme = dark_themes[dark_idx];
            }
            const LcsTheme& style = ui::get_theme(cfg.dark_theme);
            _color_buttons(style);
        }
        TablePair(Field("Rounded Corners"));
        if (ImGui::SliderInt(
                "##Rounded Corners", &cfg.rounded_corners, 0, 20)) {
            cfg.is_applied = false;
        }
        TablePair(Field("Theme Preference"));
        if (ImGui::Combo(
                "##ThemePreference", (int*)&cfg.preference, pref_table, 3)) {
            cfg.is_applied = false;
        }
        TablePair(Field("Start in Fullscreen"));
        if (ImGui::Checkbox("##Fullscreen", &cfg.start_fullscreen)) {
            cfg.is_applied = false;
        };
        ImGui::BeginDisabled(cfg.start_fullscreen);
        TablePair(Field("Startup Window"));
        Point p = { static_cast<int16_t>(cfg.startup_win_x),
            static_cast<int16_t>(cfg.startup_win_y) };
        if (PositionSelector(p, "##StartupWindow")) {
            cfg.startup_win_x = p.x;
            cfg.startup_win_y = p.y;
            cfg.is_applied    = false;
        }
        ImGui::EndDisabled();
        ImGui::EndTable();
    }
    EndSection();
    ImGuiIO& imio = ImGui::GetIO();
    ImGui::Text(
        "FPS: %4.f Uptime: %4.f seconds", imio.Framerate, ImGui::GetTime());
    ImGui::BeginDisabled(cfg.is_applied);
    if (IconButton<NORMAL>(ICON_LC_REDO_DOT, "Apply")) {
        cfg.is_saved = false;
        set_config(cfg);
        cfg            = get_config();
        cfg.is_applied = true;
        Toast(ICON_LC_SETTINGS_2, "Preferences",
            "Configuration changes were applied.");
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
        L_DEBUG("Configuration changes were saved.");
        Toast(ICON_LC_SAVE, "Preferences", "Configuration changes were saved.");
    }
    ImGui::EndDisabled();
    ImGui::End();
    if (!keep) {
        if (!get_config().is_saved) {
            L_DEBUG("Configuration changes were reverted.");
            Toast(ICON_LC_UNDO, "Preferences",
                "Configuration changes were reverted.");
            set_config(load_config());
            cfg = get_config();
        }
        _show_pref = false;
    }
}

} // namespace lcs::ui::layout
