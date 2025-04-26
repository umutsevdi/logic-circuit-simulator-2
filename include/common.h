#pragma once
/*******************************************************************************
 * \file
 * File: common.h
 * Created: 02/04/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#define APPNAME "LCS"
#define APPNAME_LONG "LogicCircuitSimulator"

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

#define VERSION 1

namespace lcs {
/**
 * A custom container class that stores a pointer to an object defined
 * within a scene. Can not be stored, copied or assigned.
 *
 * Intended use case:
 * get_node<lcs::Gate>(id)->signal();
 * get_node<lcs::Input>(id)->toggle();
 */
template <typename T> class NRef {
public:
    NRef(T* _v)
        : v(_v) { };
    NRef(NRef&&)                 = delete;
    NRef(const NRef&)            = delete;
    NRef& operator=(NRef&&)      = delete;
    NRef& operator=(const NRef&) = delete;
    ~NRef() { }

    bool operator==(void* t) const { return v == t; };
    bool operator!=(void* t) const { return v != t; };
    T* operator->() { return v; }
    T* raw(void) const { return v; }

    friend std::ostream& operator<<(std::ostream& os, const NRef<T>& g)
    {
        if (g.v != nullptr) {
            os << "NRef(" << (*g.v) << ")";
        } else {
            os << "NRef(null)";
        }
        return os;
    }

private:
    T* v;
};

enum error_t {
    OK,
    INVALID_NODEID,
    INVALID_RELID,
    REL_NOT_FOUND,
    INVALID_FROM_TYPE,
    NOT_A_COMPONENT,
    INVALID_TO_TYPE,
    ALREADY_CONNECTED,
    NOT_CONNECTED,
    COMPONENT_NOT_FOUND,
    INVALID_NODE,
    INVALID_INPUT,
    INVALID_GATE,
    INVALID_SCENE,
    INVALID_COMPONENT,
    PARSE_ERROR,
    NODE_NOT_FOUND,
    REL_CONNECT_ERROR,
    INVALID_CONTEXT,
    INVALID_DEPENDENCY_FORMAT,
    UNDEFINED_DEPENDENCY,
    INVALID_JSON_FORMAT,
    CYCLIC_DEPENDENCY,

    WRITE_FMT
};

} // namespace lcs

std::vector<std::string> split(std::string& s, const std::string& delimiter);
std::vector<std::string> split(std::string& s, const char delimiter);

extern std::string TMP;
extern std::string USER_DATA;
extern std::string LIBRARY;
extern std::string INSTALL_PATH;
void init_paths(void);

bool is_available(const std::string& author, const std::string& name,
    const std::string& version);
bool write_component(const std::string& author, const std::string& name,
    const std::string& version, const std::string& data);
std::string read_component(const std::string& author, const std::string& name,
    const std::string& version);


/******************************************************************************
                                  LOGGING/
******************************************************************************/

int __expect(std::function<bool(void)> expr, const char* msg) noexcept;
int __expect_with_message(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept;
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

inline std::string strlimit(const std::string& input, size_t limit)
{
    size_t len = input.length();
    if (len > limit) { return "..." + input.substr(len - limit+3, len); }
    return input;
}
inline std::ostream& _log_pre(std::ostream& os, const char* status,
    const char* file_name, int line, const char* function)
{
    std::ostringstream oss {};
    oss << strlimit(file_name, 20) << ":" << std::left <<std::setw(3) <<line;
    std::ostringstream oss2 {};
    oss2 << UNDERLINE BLUE << strlimit(function,20) << "()" << RESET;
    os << BOLD << status << std::left << std::setw(24) << oss.str()
       << GREEN " | " RESET << std::setw(35) << oss2.str() << GREEN " | " RESET;
    return os;
}

#define LOG_PRE(STATUS)                                                        \
    _log_pre(std::cout, (STATUS), __FILE_NAME__, __LINE__, __FUNCTION__) <<

#define CLASS *this << " "

#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (__expect_with_message([&]() mutable -> bool { return expr; },      \
                __FUNCTION__, __FILE_NAME__, __LINE__, #expr))                 \
            exit(1);                                                           \
    }
#ifndef NDEBUG
#define L_INFO(...) LOG_PRE(GREEN "INFO  | " RESET) __VA_ARGS__ << std::endl
#define L_WARN(...) LOG_PRE(GREEN "WARN  | " RESET) __VA_ARGS__ << std::endl
#define L_ERROR(...) LOG_PRE(RED "ERROR | " RESET) __VA_ARGS__ << std::endl
#else
#define L_INFO(...)
#endif
#define S_ERROR(msg, ...) (L_ERROR(msg)), __VA_ARGS__
#define ERROR(err) (L_ERROR(#err)), err
#define VEC_TO_STR(os, vec, ...)                                               \
    for (const auto& iter : vec) {                                             \
        os << __VA_ARGS__(iter) << ",\t";                                      \
    }

/******************************************************************************
                                  /LOGGING
******************************************************************************/

