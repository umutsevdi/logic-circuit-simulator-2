#include <IconsLucide.h>
#include <clocale>
#include <filesystem>
#include <json/reader.h>
#include <libintl.h>
#include "common.h"
#include "configuration.h"
#include "imgui_internal.h"
#include "json/value.h"

namespace lcs {

template <>
const char* to_str<ui::Configuration::ThemePreference>(
    ui::Configuration::ThemePreference t)
{
    switch (t) {
    case ui::Configuration::ALWAYS_LIGHT: return "light";
    case ui::Configuration::ALWAYS_DARK: return "dark";
    default: return "os";
    }
}

namespace ui {
    static void _set_locale(const std::string& locale);

    static Configuration::ThemePreference str_to_ThemePreference(
        const std::string& s)
    {
        if (s == "light") {
            return Configuration::ALWAYS_LIGHT;
        } else if (s == "dark") {
            return Configuration::ALWAYS_DARK;
        }
        return Configuration::FOLLOW_OS;
    }

    Configuration _config;

    Configuration& load_config(void)
    {
        std::string data;
        if (!fs::read(fs::ROOT / "config.json", data)) {
            L_WARN("Configuration file was not found at %s. Initializing "
                   "defaults.",
                (fs::ROOT / "config.json").c_str());
            fs::write(
                fs::ROOT / "config.json", _config.to_json().toStyledString());
            return _config;
        }

        Json::Value v;
        Json::Reader r;
        if (!r.parse(data, v)) {
            L_ERROR("Invalid configuration file format. Overwriting.");
            fs::write(
                fs::ROOT / "config.json", _config.to_json().toStyledString());
            return _config;
        }
        if (_config.from_json(v) != Error::OK) {
            L_ERROR("Parse error.");
            _config = Configuration();
        }
        L_DEBUG("Configuration was loaded.");
        _set_locale(_config.language);
        return _config;
    }

    Configuration& get_config(void) { return _config; }

    void set_config(const Configuration& cfg)
    {
        if (cfg.language != _config.language) {
            L_INFO("Update locale to %s", cfg.language.c_str());
            _set_locale(cfg.language);
        }
        _config            = cfg;
        _config.is_applied = false;
        _config.is_saved   = false;
    }

    void save_config(void)
    {
        fs::write(fs::ROOT / "config.json", _config.to_json().toStyledString());
        _config.is_applied = true;
        _config.is_saved   = true;
    }

    Json::Value Configuration::to_json() const
    {
        Json::Value v;
        v["theme"]["light"]       = light_theme;
        v["theme"]["dark"]        = dark_theme;
        v["theme"]["prefer"]      = to_str<ThemePreference>(preference);
        v["theme"]["corners"]     = rounded_corners;
        v["scale"]                = scale;
        v["language"]             = language;
        v["window"]["x"]          = startup_win_x;
        v["window"]["y"]          = startup_win_y;
        v["proxy"]                = api_proxy;
        v["window"]["fullscreen"] = start_fullscreen;
        return v;
    }

    LCS_ERROR Configuration::from_json(const Json::Value& v)
    {
        if (!(v["theme"].isObject()
                && (v["theme"]["light"].isString()
                    && v["theme"]["dark"].isString()
                    && v["theme"]["prefer"].isString()
                    && v["theme"]["corners"].isUInt())
                && v["scale"].isInt() && v["language"].isString()
                && v["window"].isObject()
                && (v["window"]["x"].isInt() && v["window"]["y"].isInt()
                    && v["window"]["fullscreen"].isBool())
                && v["proxy"].isString())) {
            return ERROR(INVALID_JSON);
        }

        light_theme = v["theme"]["light"].asString();
        dark_theme  = v["theme"]["dark"].asString();
        preference  = str_to_ThemePreference(v["theme"]["prefer"].asString());
        rounded_corners  = v["theme"]["corners"].asInt();
        scale            = v["scale"].asInt();
        api_proxy        = v["proxy"].asString();
        language         = v["language"].asString();
        startup_win_x    = v["window"]["x"].asInt();
        startup_win_y    = v["window"]["y"].asInt();
        start_fullscreen = v["window"]["fullscreen"].asBool();
        is_saved         = true;

        if (!(!light_theme.empty() && !dark_theme.empty()
                && (rounded_corners >= 0 && rounded_corners <= 20)
                && (scale >= 75 && scale <= 150))) {
            return ERROR(INVALID_JSON);
        }
        return Error::OK;
    }

    static void _set_locale(const std::string& locale)
    {
        const char* domain = APPNAME_BIN;
        std::string dir    = fs::LOCALE.string();
        if (!(bindtextdomain(domain, dir.c_str())
                && bind_textdomain_codeset(domain, "UTF-8"))) {
            L_ERROR("Error while updating translations.");
            return;
        }

        if (setlocale(LC_ALL, (locale + ".UTF-8").c_str()) == nullptr) {
            L_WARN("setlocale failed for %s", locale.c_str());
        }

        if (
#ifdef _WIN32
            _putenv_s("LC_ALL", locale.c_str())

#else
            setenv("LC_ALL", locale.c_str(), 1)
#endif
        ) {
            L_WARN("Not all LOCALE environment variables were updated.");
        }
        if (!textdomain(domain)) {
            L_WARN("textdomain failed");
        };
    }

    UserData user_data {
        .palette    = true,
        .inspector  = true,
        .scene_info = true,
        .console    = true,
        .login {},
    };

    void _apply_all(ImGuiContext*, ImGuiSettingsHandler*)
    {
        //  if (!net::get_flow().start_existing()) {
        //      Toast(ICON_LC_GITHUB,
        //          ("Welcome back " + net::get_account().name).c_str(),
        //          "Authentication was successful.");
        //      L_INFO("Welcome back %s", net::get_account().name.c_str());
        //  } else {
        //      net::get_flow().resolve();
        //  }
    }

    static void* _read_open(
        ImGuiContext*, ImGuiSettingsHandler*, const char* name)
    {
        if (std::strncmp(name, "default", sizeof("default")) == 0) {
            return &user_data;
        }
        return nullptr;
    }

    static void _read_line(
        ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
    {
        UserData* lo    = (UserData*)entry;
        uint32_t layout = 0;
        if (sscanf(line, "layout=0x%X", &layout) == 1) {
            user_data.palette    = layout & 0b0001;
            user_data.inspector  = layout & 0b0010;
            user_data.scene_info = layout & 0b100;
            user_data.console    = layout & 0b1000;
        }
        if (sscanf(line, "login=\"%127[^\"]\"", lo->login.data()) == 1) { }
    }

    static void _write_all(
        ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf)
    {
        buf->appendf("[%s][%s]\n", APPNAME_LONG, "default");
        uint32_t layout = user_data.palette
            | (user_data.inspector ? 1u : 0u << 1) | (user_data.scene_info << 2)
            | (user_data.console << 3);
        buf->appendf("layout=0x%X\n", layout);
        buf->appendf("login=\"%s\"\n\n", user_data.login.begin());
    }

    void bind_config(ImGuiContext* ctx)
    {
        ImGuiSettingsHandler handler {};
        handler.TypeName   = APPNAME_LONG;
        handler.TypeHash   = ImHashStr(APPNAME_LONG);
        handler.ReadOpenFn = _read_open;
        handler.ReadLineFn = _read_line;
        handler.WriteAllFn = _write_all;
        handler.ApplyAllFn = _apply_all;
        handler.UserData   = nullptr;
        ctx->SettingsHandlers.push_back(handler);
        L_DEBUG("Bind .ini completed.");
    }

} // namespace ui
} // namespace lcs
