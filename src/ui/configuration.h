#pragma once
/*******************************************************************************
 * \file
 * File: include/configuration.h
 * Created: 06/24/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License:
 ******************************************************************************/

#include <imgui.h>
#include <json/json.h>
#include "common.h"
#include "core.h"

namespace lcs::ui {

struct LcsTheme {
    std::string name = "";
    bool is_dark     = false;
    ImVec4 bg;
    ImVec4 fg;
    ImVec4 black;
    ImVec4 red;
    ImVec4 green;
    ImVec4 yellow;
    ImVec4 blue;
    ImVec4 magenta;
    ImVec4 cyan;
    ImVec4 white;
    ImVec4 black_bright;
    ImVec4 red_bright;
    ImVec4 green_bright;
    ImVec4 yellow_bright;
    ImVec4 blue_bright;
    ImVec4 magenta_bright;
    ImVec4 cyan_bright;
    ImVec4 white_bright;

    Json::Value to_json() const;
    LCS_ERROR from_json(const Json::Value&);
};
const LcsTheme& get_theme(const std::string& s);
const LcsTheme& get_active_style(void);
const std::vector<const char*>& get_available_styles(bool is_dark);

inline ImVec4 NodeType_to_color(Node::Type type)
{
    const LcsTheme& style = get_active_style();
    switch (type) {
    case Node::Type::GATE: return style.red;
    case Node::Type::INPUT: return style.green;
    case Node::Type::OUTPUT: return style.yellow;
    case Node::Type::COMPONENT: return style.magenta;
    default: return style.cyan;
    }
}

struct UserData {
    bool palette;
    bool inspector;
    bool scene_info;
    bool console;
    std::array<char, 128> login;
};
extern UserData user_data;

/** Embed and sync the UserData struct to ImGUIs's configuration file. */
void bind_config(ImGuiContext*);

class Configuration {
public:
    enum ThemePreference {
        FOLLOW_OS,
        ALWAYS_LIGHT,
        ALWAYS_DARK,
    };
    Configuration()  = default;
    ~Configuration() = default;

    bool is_saved              = true;
    bool is_applied            = true;
    std::string light_theme    = "Default (Light)";
    std::string dark_theme     = "Default (Dark)";
    ThemePreference preference = ThemePreference::FOLLOW_OS;
    int rounded_corners        = 0;
    int scale                  = 100;
    std::string api_proxy      = API_ENDPOINT;
    std::string language       = "en_US";
    int startup_win_x          = 1980;
    int startup_win_y          = 1080;
    bool start_fullscreen      = true;

    Json::Value to_json() const;
    LCS_ERROR from_json(const Json::Value&);
};

Configuration& get_config(void);
void set_config(Configuration&);
void save_config(void);
Configuration& load_config(void);
} // namespace lcs::ui
