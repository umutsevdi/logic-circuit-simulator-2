#include "common.h"
#include "port.h"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
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
        std::string fn = name;
        size_t pos     = 0;
        while ((pos = fn.find("virtual ")) != std::string::npos) {
            fn.erase(pos, /* virtual */ 8);
        }

        while ((pos = fn.find("lcs::")) != std::string::npos) {
            fn.erase(pos, /* lcs:: */ 5);
        }
        // Trim return type
        fn = fn.substr(fn.find_first_of(" ") + 1);

        size_t fn_end      = fn.find_first_of('(');
        size_t class_begin = 0;
        fn                 = fn.substr(class_begin, fn_end - class_begin);

        size_t fn_begin  = fn.find_last_of("::") + 1;
        fn_end           = fn.size() - 1;
        size_t class_end = fn.find_first_of("::", class_begin);
        if (fn_begin == std::string::npos || class_begin == std::string::npos) {
            // No class found
            class_end = class_begin;
            fn_begin  = class_begin;
        }

        std::string name_fn = fn.substr(fn_begin, fn_end - fn_begin + 1);
        std::string name_class
            = fn.substr(class_begin, class_end - class_begin);

        if (name_class == name_fn) {
            name_class = "lcs";
        }
        line_cache[name] = name_class + " " + name_fn;
        _fn_parse(name);
    }
}

namespace fs {

    bool ready = false;
    bool is_testing;
    std::filesystem::path ROOT;
    std::filesystem::path TMP;
    std::filesystem::path LIBRARY;
    std::filesystem::path CACHE;
    std::filesystem::path MISC;
    std::string INI;
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
        // TODO
#elif defined(__linux__)
        TMP  = "/tmp/" APPNAME_LONG;
        ROOT = home ? std::string(home) + "/.local/share/" APPNAME_LONG
                    : "/tmp/" APPNAME_LONG;
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
            __TEST_LOG__ = fopen((ROOT / "log.txt").c_str(), "w");
            L_DEBUG("Creating testing environment at %s", ROOT.c_str());
        }
        LIBRARY = ROOT / "pkg";
        CACHE   = ROOT / ".cache";
        INI     = ROOT / "profile.ini";
        MISC    = ROOT / "misc";
        try {
            if (!std::filesystem::exists(TMP)) {
                L_DEBUG("Creating %s directory.", TMP.c_str());
                std::filesystem::create_directories(TMP);
            }
            if (!std::filesystem::exists(MISC)) {
                L_DEBUG("Creating %s directory.", MISC.c_str());
                std::filesystem::create_directories(MISC);
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
                lcs::write(INI, _default_ini);
                L_DEBUG("Placing default layout");
            }
            L_DEBUG(APPNAME_LONG " file system is ready.");
        } catch (const std::exception& e) {
            L_ERROR("Directory creation failed. %s ", e.what());
        }
    }

#define F_BOLD "\033[1m"
#define F_UNDERLINE "\033[4m"
#define F_RED "\033[31m"
#define F_GREEN "\033[32m"
#define F_BLUE "\033[34m"

#define F_RESET "\033[0m"
#ifdef _WIN32
#define TEAL "\033[34m"
#else
#define F_TEAL "\033[96m"
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

        printf(F_BLUE "[%s] " F_BOLD "%s%-6s" F_RESET F_GREEN "|" F_RESET
                      "%-20s" F_GREEN " |%-18s|" F_RESET F_TEAL
                      "%-25s" F_RESET F_GREEN "|" F_RESET "%s\r\n",
            l.time_str.begin(),
            (l.severity == Message::ERROR         ? F_RED
                    : l.severity == Message::INFO ? F_GREEN
                                                  : F_TEAL),
            l.log_level_str.begin(), l.file_line.begin(), l.obj.begin(),
            l.fn.begin(), l.expr.begin());
        if (is_testing) {
            fprintf(__TEST_LOG__, "[%s] %-6s | %-20s | %-18s | %-25s | %s\r\n",
                l.time_str.begin(), l.log_level_str.begin(),
                l.file_line.begin(), l.obj.begin(), l.fn.begin(),
                l.expr.begin());
        }
    }

    void close(void)
    {
        L_INFO("Closing module lcs::fs.");
        if (__TEST_LOG__ != nullptr) {
            fclose(__TEST_LOG__);
        }
    }

    void iterate_logs(std::function<void(size_t, const Message& l)> fn)
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

    int __expect(std::function<bool(void)> expr, const char* function,
        const char* file, int line, const char* str_expr) noexcept
    {
        static const char* _title = "Logic Circuit Simulator";
        std::stringstream s {};
        s << "ERROR " << file << " : " << line << "\t" << function
          << "(...) Assertion " << str_expr << " failed!" << std::endl;
        try {
            if (expr()) {
                return 0;
            }
            tinyfd_messageBox(_title, s.str().c_str(), "ok", "error", 1);

        } catch (const std::exception& ex) {
            tinyfd_messageBox(_title, ex.what(), "ok", "error", 0);
        } catch (const std::string& ex) {
            tinyfd_messageBox(_title, ex.c_str(), "ok", "error", 0);
        }

        _log(Message { Message::ERROR, file, line, function,
            "Assertion %s failed!", str_expr });
        return 1;
    }
} // namespace fs

bool write(const std::string& path, const std::string& data)
{
    L_DEBUG("Save %s.", path.c_str());
    try {
        std::filesystem::create_directories(std::string {
            path.begin(), path.begin() + path.find_last_of("/") });
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

bool write(const std::string& path, std::vector<unsigned char>& data)
{
    L_DEBUG("Save %s.", path.c_str());
    try {
        std::filesystem::create_directories(std::string {
            path.begin(), path.begin() + path.find_last_of("/") });
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

bool read(const std::string& path, std::string& data)
{
    L_DEBUG("Opening %s.", path.c_str());
    std::filesystem::path filepath = std::filesystem::path(path);
    std::ifstream infile { filepath };
    std::string content((std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());
    data = content;
    return !content.empty();
}

bool read(const std::string& path, std::vector<unsigned char>& data)
{
    L_DEBUG("Opening %s.", path.c_str());
    std::filesystem::path filepath = std::filesystem::path(path);
    std::ifstream infile { filepath, std::ios::binary };
    std::vector<unsigned char> buffer(
        std::istreambuf_iterator<char>(infile), {});
    data = std::move(buffer);
    return !data.empty();
}

} // namespace lcs
