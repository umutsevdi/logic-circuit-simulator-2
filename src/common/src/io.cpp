#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include "common.h"

namespace lcs {
std::chrono::time_point<std::chrono::steady_clock> app_start_time;
void Message::_set_time(void)
{
    using namespace std::chrono;
    uint64_t total
        = duration_cast<milliseconds>(steady_clock::now() - app_start_time)
              .count();
    uint16_t hour = static_cast<uint16_t>(total / 3'600'000ULL); // 60*60*1000
    uint8_t min   = static_cast<uint8_t>((total % 3'600'000ULL) / 60'000ULL);
    uint8_t sec   = static_cast<uint8_t>((total % 60'000ULL) / 1'000ULL);
    uint8_t ms    = static_cast<uint8_t>(total % 1'000ULL / 10);
    if (hour == 0) {
        std::snprintf(time_str.data(), time_str.max_size() - 1,
            "%02d:%02d:%02d", min, sec, ms);
    } else {
        std::snprintf(time_str.data(), time_str.max_size() - 1,
            "%02d:%02d:%02d", hour, min, sec);
    }
}

void Message::_fn_parse(const char* name)
{
    static std::map<const char*, std::string> line_cache {};
    if (const auto& p = line_cache.find(name); p != line_cache.end()) {
        size_t class_end = p->second.find_first_of(' ');
        std::strncpy(obj.data(), p->second.data(),
            std::min(obj.max_size() - 1, class_end));
        std::strncpy(fn.data(), p->second.data() + class_end + 1,
            std::min(fn.max_size() - 1, p->second.size() - class_end - 1));
    } else {
        std::string fnname = name;
        size_t pos         = 0;
        while ((pos = fnname.find("virtual ")) != std::string::npos) {
            fnname.erase(pos, /* virtual */ 8);
        }
        while ((pos = fnname.find("__cdecl ")) != std::string::npos) {
            fnname.erase(pos, /* __cdecl */ 8);
        }
        while ((pos = fnname.find("enum ")) != std::string::npos) {
            fnname.erase(pos, /* enum */ 5);
        }

        while ((pos = fnname.find("lcs::")) != std::string::npos) {
            fnname.erase(pos, /* lcs:: */ 5);
        }
        // Trim return type
        fnname = fnname.substr(fnname.find_first_of(" ") + 1);

        if (fnname.find("lambda") == std::string::npos) {
            size_t fn_end      = fnname.find_first_of('(');
            size_t class_begin = 0;
            fnname = fnname.substr(class_begin, fn_end - class_begin);

            size_t fn_begin  = fnname.find_last_of("::") + 1;
            fn_end           = fnname.size() - 1;
            size_t class_end = fnname.find_last_of("::") - 1;
            if (fn_begin == std::string::npos
                || class_begin == std::string::npos) {
                // No class found
                class_end = class_begin;
                fn_begin  = class_begin;
            }

            std::string name_fn
                = fnname.substr(fn_begin, fn_end - fn_begin + 1);
            std::string name_class
                = fnname.substr(class_begin, class_end - class_begin);

            if (name_class == name_fn) {
                name_class = "lcs";
            }
        line_cache[name] = name_class + " " + name_fn;
        }else {
        line_cache[name] = " lambda";
        }

        printf("%s\n", line_cache[name].c_str());
        _fn_parse(name);
    }
}

namespace fs {

#include "po.h"
    const char** locales(void) { return __LOCALES__; }
    const char** localnames(void) { return __NAMES__; }
    size_t localsize(void) { return __LOCALE_S; }

    bool ready = false;
    bool is_testing;
    std::filesystem::path ROOT;
    std::filesystem::path TMP;
    std::filesystem::path LIBRARY;
    std::filesystem::path CACHE;
    std::filesystem::path MISC;
    std::filesystem::path LOCALE;

    std::filesystem::path INI;
    FILE* __TEST_LOG__ = nullptr;

    static constexpr int LINE_SIZE = 200;
    static std::array<Message, LINE_SIZE> _buffer {};
    // next item slot to write
    static size_t _next = 0;
    static size_t _size = 0;

    void init(bool _is_testing)
    {
        if (ready) {
            throw "Filesystem is already initialized";
        }
        ready            = true;
        is_testing       = _is_testing;
        app_start_time   = std::chrono::steady_clock::now();
        const char* home = std::getenv("HOME");
#ifdef _WIN32
        ROOT = std::filesystem::path { getenv("LOCALAPPDATA") } / APPNAME_BIN;
        TMP  = std::filesystem::path { getenv("TEMP") } / APPNAME_BIN;
#elif defined(__linux__)
        TMP  = "/tmp/" APPNAME_BIN;
        ROOT = home ? std::string(home) + "/.local/share/" APPNAME_BIN
                    : "/tmp/" APPNAME_BIN;

        std::string desktopfile = std::string(home)
            + "/.local/share/applications/" APPNAME_LONG ".desktop";
        if (!std::filesystem::exists(desktopfile)) {
            std::printf("Desktop file does not exist\r\n");
            std::string data =
#include "lcs.desktop.txt"
                ;
            std::printf("APPNAME\r\n");
            data = data.replace(data.find("%APPNAME%"), 9, APPNAME_BIN);
            std::printf("HOME\r\n");
            data = data.replace(data.find("%HOME%"), 6, home);
            std::printf("APPNAME : %s\r\n", data.c_str());
            data = data.replace(data.find("%APPNAME%"), 9, APPNAME_BIN);
            write(desktopfile, data);
        }
#elif defined(__unix__)
        // TODO For BSD & Mac
#endif
        if (is_testing) {
            time_t now = time(nullptr);
            tm* t      = localtime(&now);
            std::stringstream s_time { ".test_" };
            s_time << "run" << "_" << t->tm_hour << "_" << t->tm_min << "_"
                   << t->tm_sec;
            s_time <<
#ifdef NDEBUG
                "_rel"
#else
                "_dbg"
#endif
                ;
            ROOT = TMP / s_time.str();
            TMP  = TMP / s_time.str() / "tmp";
            if (!std::filesystem::exists(ROOT)) {
                std::filesystem::create_directories(ROOT);
            }
            auto logfile = ROOT / "log.txt";
#ifdef _MSC_VER
            __TEST_LOG__ = _wfopen(logfile.c_str(), L"w");
#else
            __TEST_LOG__ = std::fopen(logfile.c_str(), "w");
#endif

            L_DEBUG("Creating testing environment at %s", ROOT.c_str());
        }
        LIBRARY = ROOT / "pkg";
        CACHE   = ROOT / ".cache";
        INI     = ROOT / "profile.ini";
        MISC    = ROOT / "misc";
        LOCALE  = MISC / "locale";
        try {
            if (!std::filesystem::exists(TMP)) {
                L_DEBUG("Creating %s directory.", TMP.c_str());
                std::filesystem::create_directories(TMP);
            }
            if (!std::filesystem::exists(MISC)) {
                L_DEBUG("Creating %s directory.", MISC.c_str());
                std::filesystem::create_directories(MISC);
            }
            if (!std::filesystem::exists(LOCALE)) {
                L_DEBUG("Creating %s directory.", LOCALE.c_str());
                std::filesystem::create_directories(LOCALE);
            }
            if (!std::filesystem::exists(LIBRARY)) {
                L_DEBUG("Creating %s directory.", LIBRARY.c_str());
                std::filesystem::create_directories(LIBRARY);
            }
            if (!std::filesystem::exists(CACHE)) {
                L_DEBUG("Creating %s directory.", CACHE.c_str());
                std::filesystem::create_directories(CACHE);
            }
            if (!std::filesystem::exists(INI)) {
                static const char* _default_ini =
#include "default_ini.txt"
                    ;
                fs::write(INI, _default_ini);
                L_DEBUG("Placing default layout");
            }
        } catch (const std::exception& e) {
            L_ERROR("Directory creation failed. %s ", e.what());
        }
        for (size_t i = 0; i < localsize(); i++) {
            if (std::filesystem::is_directory(LOCALE / locales()[i])) {
                L_DEBUG("Discovered %s(%s)", locales()[i], localnames()[i]);
            } else {
                L_WARN("Locale %s not found!", locales()[i]);
            }
        }
        L_INFO("Module lcs::fs is ready");
    }

#ifndef _WIN32
#define F_BOLD "\033[1m"
#define F_UNDERLINE "\033[4m"
#define F_RED "\033[31m"
#define F_GREEN "\033[32m"
#define F_BLUE "\033[34m"
#define F_INVERT "\033[7m"
#define F_RESET "\033[0m"
#define F_TEAL "\033[96m"
#else
#define F_BOLD ""
#define F_UNDERLINE ""
#define F_RED ""
#define F_GREEN ""
#define F_BLUE ""
#define F_INVERT ""
#define F_RESET ""
#define F_TEAL ""
#endif

    void _log(const Message& l)
    {

#ifdef NDEBUG
        if (Message::DEBUG == l.severity) {
            return;
        }
#endif
        if (_size < LINE_SIZE) {
            _buffer[_next] = l;
            _next++;
            _size++;
        } else {
            _next          = (_next + 1) % _size;
            _buffer[_next] = l;
        }

        const char* clr;
        switch (l.severity) {
        case Message::FATAL:
        case Message::ERROR: clr = F_RED; break;
        case Message::INFO: clr = F_GREEN; break;
        default: clr = F_TEAL; break;
        }

        printf(F_BLUE "[%s] " F_BOLD "%s%-6s" F_RESET F_GREEN "|" F_RESET
                      "%-20s" F_GREEN " |%-18s|" F_RESET F_TEAL
                      "%-25s" F_RESET F_GREEN "|" F_RESET "%s%s\r\n" F_RESET,
            l.time_str.data(), clr, l.log_level_str.data(), l.file_line.data(),
            l.obj.data(), l.fn.data(),
            l.severity == Message::FATAL ? F_INVERT F_BOLD F_RED : "",
            l.expr.data());
        if (is_testing) {
            fprintf(__TEST_LOG__, "[%s] %-6s | %-20s | %-18s | %-25s | %s\r\n",
                l.time_str.data(), l.log_level_str.data(), l.file_line.data(),
                l.obj.data(), l.fn.data(), l.expr.data());
        }
    }

    void close(void)
    {
        if (__TEST_LOG__ != nullptr) {
            fclose(__TEST_LOG__);
        }
        L_INFO("Module lcs::fs is closed.");
    }

    void logs_for_each(std::function<void(size_t, const Message& l)> fn)
    {
        if (_size < LINE_SIZE) {
            for (size_t i = 0; i < _size; i++) {
                fn(i, _buffer[i]);
            }
        } else {
            // if the buffer is full. The item we're about the write into is
            // the oldest
            size_t idx = (_next) % _size;
            while (idx != (_next - 1) % LINE_SIZE) {
                fn(idx, _buffer[idx]);
                idx = (idx + 1) % _size;
            }
        }
    }

    void clear_log(void)
    {
        _next = 0;
        _size = 0;
    }

    bool write(const std::filesystem::path& path, const std::string& data)
    {
        L_DEBUG("Save %s.", path.c_str());
        try {
            std::filesystem::create_directories(path.parent_path());
            std::ofstream outfile { path };
            if (outfile) {
                outfile << data;
                return true;
            }
            L_ERROR("Failed to open file for writing %s.", path.c_str());
        } catch (const std::exception& e) {
            L_ERROR("Exception occurred while writing %s.", e.what());
        }
        return false;
    }

    bool write(
        const std::filesystem::path& path, std::vector<unsigned char>& data)
    {
        L_DEBUG("Save %s.", path.c_str());
        try {
            std::filesystem::create_directories(path.parent_path());
            std::ofstream outfile { path, std::ios::binary };
            if (outfile) {
                outfile.write((char*)data.data(), data.size());
                return true;
            }
            L_ERROR("Failed to open file for writing %s.", path.c_str());
        } catch (const std::exception& e) {
            L_ERROR("Exception occurred while writing %s.", e.what());
        }
        return false;
    }

    bool read(const std::filesystem::path& path, std::string& data)
    {
        L_DEBUG("Opening %s.", (char*)path.c_str());
        std::ifstream infile { path };
        std::string content((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        data = content;
        return !content.empty();
    }

    bool read(
        const std::filesystem::path& path, std::vector<unsigned char>& data)
    {
        L_DEBUG("Opening %s.", path.c_str());
        std::ifstream infile { path, std::ios::binary };
        std::vector<unsigned char> buffer(
            std::istreambuf_iterator<char>(infile), {});
        data = std::move(buffer);
        return !data.empty();
    }

} // namespace fs
} // namespace lcs
