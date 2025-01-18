#include "common.h"
#include <sstream>
#include <string>
#include <tinyfiledialogs.h>

int __expect(std::function<bool(void)> expr, const char* msg) noexcept
{
    int is_err = 1;
    try {
        if (expr())
            is_err = 0;
        else
            tinyfd_messageBox("Logic Circuit Simulator: Assertion Failed", msg,
                "ok", "error", 1);
    } catch (const std::exception& ex) {
        tinyfd_messageBox(
            "Logic Circuit Simulator: Exception", ex.what(), "ok", "error", 0);
    } catch (const std::string& ex) {
        tinyfd_messageBox(
            "Logic Circuit Simulator: Exception", ex.c_str(), "ok", "error", 0);
    }
    if (is_err) { std::cout << msg; }
    return is_err;
}

int __expect_with_message(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept
{
    std::stringstream s {};

    s << "ERROR " << file << " : " << line << "\t" << function
      << "(...) Assertion " << str_expr << " failed!" << std::endl;
    return __expect(expr, s.str().c_str());
}
