#include "common.h"
#include <base64.h>
#include <json/json.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace lcs {
fs::path TMP;
fs::path ROOT;
fs::path LIBRARY;
fs::path LOCAL;
fs::path CACHE;
std::string INI;
std::ofstream __TEST_LOG__;

static std::string current_path;
bool is_testing = false;

void init_paths(bool _is_testing)
{
    is_testing       = _is_testing;
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

        __TEST_LOG__ = std::ofstream { ROOT / "log.txt" };
        L_INFO("Creating testing environment at %s", ROOT.c_str());
    }
    LIBRARY = ROOT / "lib";
    LOCAL   = ROOT / "local";
    CACHE   = ROOT / ".cache";
    INI     = ROOT / "loc.ini";
    try {
        if (!fs::exists(TMP)) {
            L_INFO("Creating %s directory.", TMP.c_str());
            fs::create_directories(TMP);
        }
        if (!fs::exists(LIBRARY)) {
            L_INFO("Creating %s directory.", LIBRARY.c_str());
            fs::create_directories(LIBRARY);
        }
        if (!fs::exists(LOCAL)) {
            L_INFO("Creating %s directory.", LOCAL.c_str());

            fs::create_directories(LOCAL);
        }
        if (!fs::exists(CACHE)) {
            L_INFO("Creating %s directory.", CACHE.c_str());
            fs::create_directories(CACHE);
        }
        if (!fs::exists(INI)) {
            static const char* _default_ini =
#include "default_ini.txt"
                ;
            lcs::write(INI, _default_ini);
            L_INFO("Placing default layout");
        }
        L_INFO(APPNAME_LONG " file system is ready.");
    } catch (const std::exception& e) {
        L_ERROR("Directory creation failed. %s ", e.what());
    }
}

bool write(const std::string& path, const std::string& data)
{
    L_INFO("Save %s.", path.c_str());
    try {
        fs::create_directories(std::string {
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
    L_INFO("Save %s.", path.c_str());
    try {
        fs::create_directories(std::string {
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

std::string read(const std::string& path)
{
    L_INFO("Reading %s.", path.c_str());
    fs::path json_path = fs::path(path);
    std::ifstream infile { json_path };
    std::string content((std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());
    return content;
}

bool read(const std::string& path, std::vector<unsigned char>& data)
{
    L_INFO("Reading %s.", path.c_str());
    fs::path json_path = fs::path(path);
    std::ifstream infile { json_path, std::ios::binary };
    std::vector<unsigned char> buffer(
        std::istreambuf_iterator<char>(infile), {});
    data = std::move(buffer);
    return !data.empty();
}
} // namespace lcs
