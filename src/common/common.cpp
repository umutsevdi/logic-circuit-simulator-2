#include <tinyfiledialogs.h>

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>

#include "common.h"

std::optional<std::ofstream> TESTLOG = std::nullopt;
static bool COLORS_ENABLED           = true;
bool is_color_enabled(void) { return COLORS_ENABLED; }

std::ostream& _log_pre(std::ostream& stream, const char* status,
    const char* file_name, int line, const char* function)
{
    COLORS_ENABLED = true;
    std::ostringstream oss {};
    oss << strlimit(file_name, 20) << ":" << std::left << std::setw(3) << line;
    std::ostringstream oss2 {};
    oss2 << F_UNDERLINE F_BLUE << strlimit(function, 20) << "()" << F_RESET;
    stream << F_BOLD << status << std::left << std::setw(24) << oss.str()
           << F_GREEN " | " F_RESET << std::setw(35) << oss2.str()
           << F_GREEN " | " F_RESET;
    return stream;
}

std::ostream& _log_pre_f(
    const char* status, const char* file_name, int line, const char* function)
{
    if (TESTLOG.has_value()) {
        COLORS_ENABLED = false;
        std::ostringstream oss {};
        oss << strlimit(file_name, 20) << ":" << std::left << std::setw(3)
            << line;
        std::ostringstream oss2 {};
        oss2 << strlimit(function, 20) << "()";
        TESTLOG.value() << status << std::left << std::setw(24) << oss.str()
                        << " | " << std::setw(35) << oss2.str() << " | ";
        return TESTLOG.value();
    }
    return std::cout;
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
    _log_pre(std::cerr, __F_FATAL, file, line, function) << F_RED F_BOLD
        "Assertion " << str_expr << " failed!" F_RESET << std::endl;
    if (TESTLOG.has_value()) {
        _log_pre_f(_FATAL, file, line, function)
            << "Assertion " << str_expr << " failed!" << std::endl;
    }
    return 1;
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
