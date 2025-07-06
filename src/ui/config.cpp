#include "common.h"
#include "core.h"
#include "io.h"
#include "json/reader.h"
#include "json/value.h"
#include "ui/configuration.h"

namespace lcs {
namespace ui {
    Configuration _config;

    const char* Style_to_str(Style style)
    {
        switch (style) {
        case SEOUL256_LIGHT: return "SEOUL256_LIGHT";
        case ACME: return "ACME";
        case GRUVBOX_LIGHT: return "GRUVBOX_LIGHT";
        case ONE_LIGHT: return "ONE_LIGHT";
        case SEASHELLS: return "SEASHELLS";
        case XTERM: return "XTERM";
        case GRUVBOX_DARK: return "GRUVBOX_DARK";
        case ONE_DARK: return "ONE_DARK";
        default: return "UNKNOWN";
        }
    }

    Style str_to_Style(const std::string& str)
    {
        if (str == "SEOUL256_LIGHT") {
            return SEOUL256_LIGHT;
        } else if (str == "ACME") {
            return ACME;
        } else if (str == "GRUVBOX_LIGHT") {
            return GRUVBOX_LIGHT;
        } else if (str == "ONE_LIGHT") {
            return ONE_LIGHT;
        } else if (str == "SEASHELLS") {
            return SEASHELLS;
        } else if (str == "XTERM") {
            return XTERM;
        }
        if (str == "GRUVBOX_DARK") {
            return GRUVBOX_DARK;
        } else if (str == "ONE_DARK") {
            return ONE_DARK;
        }
        return Style::STYLE_S;
    }

    const char* ThemePreference_to_str(Configuration::ThemePreference t)
    {
        switch (t) {
        case Configuration::ALWAYS_LIGHT: return "light";
        case Configuration::ALWAYS_DARK: return "dark";
        default: return "os";
        }
    }

    Configuration::ThemePreference str_to_ThemePreference(const std::string& s)
    {
        if (s == "light") {
            return Configuration::ALWAYS_LIGHT;
        } else if (s == "dark") {
            return Configuration::ALWAYS_DARK;
        }
        return Configuration::FOLLOW_OS;
    }
} // namespace ui

template <>
Json::Value lcs::to_json<ui::Configuration>(const ui::Configuration& c)
{
    Json::Value v;
    v["theme"]                = {};
    v["theme"]["light"]       = Style_to_str(c.light_theme);
    v["theme"]["dark"]        = Style_to_str(c.dark_theme);
    v["theme"]["prefer"]      = ThemePreference_to_str(c.preference);
    v["theme"]["corners"]     = c.rounded_corners;
    v["scale"]                = c.scale;
    v["language"]             = c.language;
    v["window"]["x"]          = c.startup_win_x;
    v["window"]["y"]          = c.startup_win_y;
    v["proxy"]                = c.api_proxy;
    v["window"]["fullscreen"] = c.start_fullscreen;
    return v;
}

template <>
LCS_ERROR lcs::from_json<ui::Configuration>(
    const Json::Value& v, ui::Configuration& c)
{
    if (!(v["theme"].isObject()
            && (v["theme"]["light"].isString() && v["theme"]["dark"].isString()
                && v["theme"]["prefer"].isString()
                && v["theme"]["corners"].isBool())
            && v["scale"].isInt() && v["language"].isString()
            && v["window"].isObject()
            && (v["window"]["x"].isInt() && v["window"]["y"].isInt()
                && v["window"]["fullscreen"].isBool())
            && v["proxy"].isString())) {
        return ERROR(INVALID_JSON_FORMAT);
    }

    c.light_theme = ui::str_to_Style(v["theme"]["light"].asString());
    c.dark_theme  = ui::str_to_Style(v["theme"]["dark"].asString());
    c.preference  = ui::str_to_ThemePreference(v["theme"]["prefer"].asString());
    c.rounded_corners  = v["theme"]["corners"].asBool();
    c.scale            = v["scale"].asInt();
    c.api_proxy        = v["proxy"].asString();
    c.language         = v["language"].asString();
    c.startup_win_x    = v["window"]["x"].asInt();
    c.startup_win_y    = v["window"]["y"].asInt();
    c.start_fullscreen = v["window"]["fullscreen"].asBool();
    c.is_saved         = true;

    if (!(c.light_theme != ui::Style::STYLE_S
            && c.dark_theme != ui::Style::STYLE_S && c.scale > 50)) {
        return ERROR(INVALID_JSON_FORMAT);
    }
    return Error::OK;
}

namespace ui {
    Configuration& load_config(void)
    {
        std::string data = read(ROOT / "config.json");
        if (data == "") {
            L_ERROR(
                "Configuration file was not found at %s", ROOT / "config.json");
            write(ROOT / "config.json",
                to_json<Configuration>(_config).toStyledString());
            return _config;
        }

        Json::Value v;
        Json::Reader r;
        if (!r.parse(data, v)) {
            L_ERROR("Invalid configuration file format. Overwriting.");
            write(ROOT / "config.json",
                to_json<Configuration>(_config).toStyledString());
            return _config;
        }
        if (from_json<Configuration>(v, _config) != Error::OK) {
            L_ERROR("Parse error.");
            _config = Configuration();
        }
        L_DEBUG("Configuration was loaded.");
        return _config;
    }

    Configuration& get_config(void) { return _config; }

    void set_config(const Configuration& cfg)
    {
        _config            = cfg;
        _config.is_applied = false;
        _config.is_saved   = false;
    }

    void save_config(void)
    {
        write(ROOT / "config.json",
            to_json<Configuration>(_config).toStyledString());
    }
} // namespace ui

} // namespace lcs
