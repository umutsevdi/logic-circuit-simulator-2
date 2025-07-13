#include "IconsLucide.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include <imgui.h>
#include <tinyfiledialogs.h>

namespace lcs::ui {

static Configuration cfg = {};

static void _color_buttons(const LcsTheme&);
void Preferences(bool& pref_show)
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
    if (!pref_show) {
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
        Point p = { cfg.startup_win_x, cfg.startup_win_y };
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
        L_INFO("Configuration changes were saved.");
        Toast(ICON_LC_SAVE, "Preferences", "Configuration changes were saved.");
    }
    ImGui::EndDisabled();
    ImGui::End();
    if (!keep) {
        if (!get_config().is_saved) {
            L_INFO("Configuration changes were reverted.");
            Toast(ICON_LC_UNDO, "Preferences",
                "Configuration changes were reverted.");
            set_config(load_config());
            cfg = get_config();
        }
        pref_show = false;
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
} // namespace lcs::ui
  //
