#include "common.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tinyfiledialogs.h>

int __expect(std::function<bool(void)> expr, const char* msg) noexcept
{
    int is_err = 1;
    try {
        if (expr()) {
            is_err = 0;
        } else {
            tinyfd_messageBox("Logic Circuit Simulator: Assertion Failed", msg,
                "ok", "error", 1);
        }
    } catch (const std::exception& ex) {
        L_ERROR("Exception occurred: " << ex.what());
        tinyfd_messageBox(
            "Logic Circuit Simulator: Exception", ex.what(), "ok", "error", 0);
    } catch (const std::string& ex) {
        L_FATAL("Exception occurred: " << ex);
        tinyfd_messageBox(
            "Logic Circuit Simulator: Exception", ex.c_str(), "ok", "error", 0);
    }
    return is_err;
}

int __expect_with_message(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept
{

    _log_pre(std::cout, __S_FATAL, file, line, function)
        << RED BOLD "Assertion " << str_expr << " failed!" RESET << std::endl;

    std::stringstream s {};
    s << "ERROR " << file << " : " << line << "\t" << function
      << "(...) Assertion " << str_expr << " failed!" << std::endl;
    return __expect(expr, s.str().c_str());
}

std::vector<std::string> split(std::string& s, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

std::vector<std::string> split(std::string& s, const char delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + 1);
    }
    tokens.push_back(s);

    return tokens;
}

namespace fs = std::filesystem;
std::string TMP;
std::string USER_DATA;
std::string LIBRARY;
std::string INSTALL_PATH;

void init_paths()
{
    const char* home = std::getenv("HOME");

#ifdef _WIN32
    const char* tmp     = std::getenv("TEMP");
    const char* appdata = std::getenv("APPDATA");
    TMP                 = tmp ? tmp : "C:\\Temp\\" APPNAME_LONG;
    USER_DATA           = appdata ? std::string(appdata) + "\\" APPNAME_LONG
                                  : "C:\\Users\\Default\\AppData\\Roaming\\" APPNAME_LONG;
    INSTALL_PATH        = "C:\\Program Files\\" APPNAME_LONG;
    LIBRARY             = USER_DATA + "\\lib";

#elif defined(__APPLE__)
    TMP          = "/tmp/" APPNAME_LONG;
    USER_DATA    = home
           ? std::string(home) + "/Library/Application Support/" APPNAME_LONG
           : "/tmp/" APPNAME_LONG;
    INSTALL_PATH = "/Applications/" APPNAME_LONG;
    LIBRARY      = USER_DATA + "/lib";

#elif defined(__linux__)
    TMP          = "/tmp";
    USER_DATA    = home ? std::string(home) + "/.local/share/" APPNAME_LONG
                        : "/tmp/" APPNAME_LONG;
    INSTALL_PATH = "/usr/share/" APPNAME_LONG;
    LIBRARY      = USER_DATA + "/lib";

#elif defined(__unix__)
    TMP          = "/tmp" APPNAME_LONG;
    USER_DATA    = home ? std::string(home) + "/.local/share/" APPNAME_LONG
                        : "/tmp/" APPNAME_LONG;
    INSTALL_PATH = "/usr/local/" APPNAME_LONG;
    LIBRARY      = USER_DATA + "/lib";
#else
    std::cerr << "Unsupported platform!" << std::endl;
#endif
    try {
        if (!fs::exists(USER_DATA)) {
            L_INFO("Creating" << USER_DATA);
            //            fs::create_directories(USER_DATA);
        }
        if (!fs::exists(LIBRARY)) {
            L_INFO("Creating" << LIBRARY);
            //            fs::create_directories(USER_DATA);
        }
        if (!fs::exists(TMP)) {
            L_INFO("Creating" << TMP);

            //    fs::create_directories(TMP);
        }
        //        if (!fs::exists(INSTALL_PATH)) {
        //        fs::create_directories(INSTALL_PATH);}
    } catch (const std::exception& e) {
        L_ERROR("Directory creation failed: " << e.what());
    }
}

namespace fs = std::filesystem;

bool is_available(const std::string& author, const std::string& name,
    const std::string& version)
{
    if (LIBRARY.empty()) {
        std::cerr << "Error: LIBRARY is not initialized." << std::endl;
        return false;
    }
    fs::path json_path
        = fs::path(LIBRARY) / author / name / (version + ".json");
    std::ifstream infile { json_path };
    if (!infile) {
        L_ERROR("Component file not found: " << json_path);
        return false;
    }
    return true;
}

bool write_component(const std::string& author, const std::string& name,
    const std::string& version, const std::string& data)
{
    if (LIBRARY.empty()) {
        std::cerr << "Error: LIBRARY is not initialized." << std::endl;
        return false;
    }

    fs::path json_path = fs::path(LIBRARY) / author / name;
    try {
        fs::create_directories(json_path);
        json_path /= version + ".json";

        std::ofstream outfile { json_path };
        if (!outfile) {
            L_ERROR("Failed to open file for writing: " << json_path);
            return false;
        }
        outfile << data;
        return true;
    } catch (const std::exception& e) {
        L_ERROR("Exception in write_component: " << e.what());
        return false;
    }
}

std::string read_component(const std::string& author, const std::string& name,
    const std::string& version)
{
    if (!is_available(author, name, version)) { return ""; }
    fs::path json_path
        = fs::path(LIBRARY) / author / name / (version + ".json");
    std::ifstream infile { json_path };
    std::string content((std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());
    return content;
}
