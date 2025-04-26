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

enum error_t {
    /** Operation is successful. */
    OK,
    /** Object has 0 as node id. */
    INVALID_NODEID,
    /** Object has 0 as rel id. */
    INVALID_RELID,
    /** Given relationship does not exist within that scene. */
    REL_NOT_FOUND,
    /** Given node does not exist within that scene. */
    NODE_NOT_FOUND,
    /** Outputs can not be used as a from type. */
    INVALID_FROM_TYPE,
    /** Only components can have node_t::COMPONENT_INPUT and
       node_t::COMPONENT_OUTPUT. */
    NOT_A_COMPONENT,
    /** Inputs can not be used as a to type. */
    INVALID_TO_TYPE,
    /** Attempted to bind a already connected input socket. */
    ALREADY_CONNECTED,
    /** Component socket is not connected. */
    NOT_CONNECTED,
    /** Attempted to execute or load a component that does not exist. */
    COMPONENT_NOT_FOUND,
    /** Deserialized node does not fulfill its requirements. */
    INVALID_NODE,
    /** Deserialized input does not fulfill its requirements. */
    INVALID_INPUT,
    /** Deserialized gate does not fulfill its requirements. */
    INVALID_GATE,
    /** Deserialized scene does not fulfill its requirements. */
    INVALID_SCENE,
    /** Deserialized component does not fulfill its requirements. */
    INVALID_COMPONENT,
    /** An error occurred while connecting a node during serialization. */
    REL_CONNECT_ERROR,
    /** Invalid dependency string. */
    INVALID_DEPENDENCY_FORMAT,
    /** Component contains a undefined dependency. */
    UNDEFINED_DEPENDENCY,
    /** Not a valid JSON document. */
    INVALID_JSON_FORMAT,
    CYCLIC_DEPENDENCY,
};

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
    NRef(NRef&&)                 = default;
    NRef(const NRef&)            = delete;
    NRef& operator=(NRef&&)      = delete;
    NRef& operator=(const NRef&) = delete;
    ~NRef() { }

    bool operator==(void* t) const { return v == t; };
    bool operator!=(void* t) const { return v != t; };
    T* operator->() { return v; }
    const T* operator->() const { return v; }
    const T* raw(void) const { return v; }
    T* raw(void) { return v; }

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
#define __S_INFO GREEN "INFO  | " RESET
#define __S_WARN GREEN "WARN  | " RESET
#define __S_ERROR RED "ERROR | " RESET
#define __S_FATAL RED "FATAL | " RESET

inline std::string strlimit(const std::string& input, size_t limit)
{
    size_t len = input.length();
    if (len > limit) { return "..." + input.substr(len - limit + 3, len); }
    return input;
}
inline std::ostream& _log_pre(std::ostream& os, const char* status,
    const char* file_name, int line, const char* function)
{
    std::ostringstream oss {};
    oss << strlimit(file_name, 20) << ":" << std::left << std::setw(3) << line;
    std::ostringstream oss2 {};
    oss2 << UNDERLINE BLUE << strlimit(function, 20) << "()" << RESET;
    os << BOLD << status << std::left << std::setw(24) << oss.str()
       << GREEN " | " RESET << std::setw(35) << oss2.str() << GREEN " | " RESET;
    return os;
}

#define LOG_PRE(STATUS)                                                        \
    _log_pre(std::cout, (STATUS), __FILE_NAME__, __LINE__, __FUNCTION__) <<

#define CLASS BOLD GREEN << *this << RESET "\t"

#define L_WARN(...) LOG_PRE(__S_WARN) __VA_ARGS__ << std::endl
#define L_ERROR(...) LOG_PRE(__S_ERROR) __VA_ARGS__ << std::endl
#define L_FATAL(...)                                                           \
    LOG_PRE(__S_FATAL) RED BOLD << __VA_ARGS__ << RESET << std::endl

#ifndef NDEBUG
#define L_INFO(...) LOG_PRE(__S_INFO) __VA_ARGS__ << std::endl
#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (__expect_with_message([&]() mutable -> bool { return expr; },      \
                __FUNCTION__, __FILE_NAME__, __LINE__, #expr))                 \
            exit(1);                                                           \
    }
#else
#define L_INFO(...)
#define lcs_assert(expr)
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
