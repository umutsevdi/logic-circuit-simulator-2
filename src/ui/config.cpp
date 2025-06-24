#include "common.h"
#include "io.h"
#include "json/reader.h"
#include "json/value.h"
#include "ui/configuration.h"

namespace lcs::ui {
Configuration _config;

void load_config(void)
{
    std::string data = io::read(io::ROOT / "config.json");
    if (data == "") {
        L_ERROR(
            "Configuration file was not found at " << io::ROOT / "config.json");
        io::write(io::ROOT / "config.json", _config.to_json().toStyledString());
        return;
    }

    Json::Value v;
    Json::Reader r;
    if (!r.parse(data, v)) {
        L_ERROR("Invalid configuration file format. Overwriting.");
        io::write(io::ROOT / "config.json", _config.to_json().toStyledString());
        return;
    }
    if (_config.from_json(v) != Error::OK) {
        L_ERROR("Parse error.");
        _config = Configuration();
    }
}

Configuration& get_config(void) { return _config; }

void save_config(void)
{
    io::write(io::ROOT / "config.json", _config.to_json().toStyledString());
}

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
    case Configuration::FOLLOW_OS: return "os";
    case Configuration::ALWAYS_LIGHT: return "light";
    case Configuration::ALWAYS_DARK: return "dark";
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
Json::Value Configuration::to_json(void) const
{
    Json::Value v;
    v["theme"]                = {};
    v["theme"]["light"]       = Style_to_str(light_theme);
    v["theme"]["dark"]        = Style_to_str(dark_theme);
    v["theme"]["prefer"]      = ThemePreference_to_str(preference);
    v["theme"]["corners"]     = rounded_corners;
    v["font"]                 = font_size;
    v["scale"]                = scale;
    v["language"]             = language;
    v["window"]["x"]          = startup_win_x;
    v["window"]["y"]          = startup_win_y;
    v["window"]["fullscreen"] = is_fullscr;
    v["proxy"]                = api_proxy;
    return v;
}

LCS_ERROR Configuration::from_json(const Json::Value& v)
{
    if (!(v["theme"].isObject()
            && (v["theme"]["light"].isString() && v["theme"]["dark"].isString()
                && v["theme"]["prefer"].isString()
                && v["theme"]["corners"].isBool())
            && v["font"].isNumeric() && v["scale"].isInt()
            && v["language"].isString() && v["window"].isObject()
            && (v["window"]["x"].isInt() && v["window"]["y"].isInt()
                && v["window"]["fullscreen"].isBool())
            && v["proxy"].isString())) {
        return ERROR(INVALID_JSON_FORMAT);
    }

    font_size       = v["font"].asFloat();
    light_theme     = str_to_Style(v["theme"]["light"].asString());
    dark_theme      = str_to_Style(v["theme"]["dark"].asString());
    preference      = str_to_ThemePreference(v["theme"]["prefer"].asString());
    rounded_corners = v["theme"]["corners"].asBool();
    scale           = v["scale"].asInt();
    api_proxy       = v["proxy"].asString();
    language        = v["language"].asString();
    startup_win_x   = v["window"]["x"].asInt();
    startup_win_y   = v["window"]["y"].asInt();
    is_fullscr      = v["window"]["fullscreen"].asBool();
    is_saved        = true;

    if (!(font_size > 6.f && light_theme != Style::STYLE_S
            && dark_theme != Style::STYLE_S && scale > 50)) {
        return ERROR(INVALID_JSON_FORMAT);
    }
    return Error::OK;
}

} // namespace lcs::ui
