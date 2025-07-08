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

#include "common.h"
#include "ui/util.h"

namespace lcs::ui {

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

    Style light_theme          = Style::SEOUL256_LIGHT;
    Style dark_theme           = Style::SEASHELLS;
    ThemePreference preference = ThemePreference::FOLLOW_OS;
    int rounded_corners        = 0;
    int scale                  = 100;
    std::string api_proxy      = API_ENDPOINT;
    std::string language       = "en_us"; // TODO
    int startup_win_x          = 1980;
    int startup_win_y          = 1080;
    bool is_saved              = true;
    bool is_applied            = true;
    bool start_fullscreen      = true;
};

Configuration& get_config(void);
void set_config(const Configuration&);
void save_config(void);
Configuration& load_config(void);

} // namespace lcs::ui
