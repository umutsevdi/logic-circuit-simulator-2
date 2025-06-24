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
#include "io.h"
#include "ui/util.h"

namespace lcs::ui {
class Configuration final : public lcs::io::Serializable {
public:
    enum ThemePreference {
        FOLLOW_OS,
        ALWAYS_LIGHT,
        ALWAYS_DARK,
    };
    Configuration()  = default;
    ~Configuration() = default;

    Json::Value to_json(void) const override;
    LCS_ERROR from_json(const Json::Value&) override;

    float font_size            = 16.0f;
    Style light_theme          = Style::SEOUL256_LIGHT;
    Style dark_theme           = Style::SEASHELLS;
    ThemePreference preference = ThemePreference::FOLLOW_OS;
    bool rounded_corners       = true;
    int scale                  = 100;
    std::string api_proxy      = API_ENDPOINT;
    std::string language       = "en_us"; // TODO
    int startup_win_x          = 1980;
    int startup_win_y          = 1080;
    bool is_fullscr            = false;
    bool is_saved              = true;
    bool is_applied            = true;
};

Configuration& get_config(void);
void save_config(void);
void load_config(void);

} // namespace lcs::ui
