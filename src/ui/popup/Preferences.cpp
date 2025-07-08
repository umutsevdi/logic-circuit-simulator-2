#include "IconsLucide.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

namespace lcs::ui {

static Configuration cfg { .is_saved = true, .is_applied = true };
void Preferences(bool& pref_show)
{
    if (!pref_show) {
        return;
    }
    if (cfg.is_saved && cfg.is_applied) {
        cfg = get_config();
    }
    bool keep = true;
    const static ImVec2 __table_l_size
        = ImGui::CalcTextSize("LIGHT THEME STYLE");
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
} // namespace lcs::ui
