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
#include <iostream>
#include <optional>

#define APPNAME "LCS"
#define APPNAME_LONG "LogicCircuitSimulator"

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
    /** Invalid file format */
    NOT_A_JSON,
    /** No such file or directory in given path */
    NOT_FOUND,
    /** Failed to save file*/
    NO_SAVE_PATH_DEFINED,
    CYCLIC_DEPENDENCY,
};

/**
 * A custom container class that stores a pointer to an object defined
 * within a scene. Can not be stored, copied or assigned.
 *
 * Intended use case:
 * get_node<lcs::GateNode>(id)->signal();
 * get_node<lcs::InputNode>(id)->toggle();
 */
template <typename T> class NRef {
public:
    NRef(T* v)
        : _v(v) { };
    NRef(NRef&&)                 = default;
    NRef(const NRef&)            = delete;
    NRef& operator=(NRef&&)      = delete;
    NRef& operator=(const NRef&) = delete;
    ~NRef() { }

    inline bool operator==(void* t) const { return _v == t; };
    inline bool operator!=(void* t) const { return _v != t; };
    inline T* operator->() { return _v; }
    inline const T* operator->() const { return _v; }
    inline T* operator&() { return _v; }

    friend std::ostream& operator<<(std::ostream& os, const NRef<T>& g)
    {
        os << "NRef(";
        if (g._v != nullptr) {
            os << *g._v;
        } else {
            os << "null";
        }
        os << ")";
        return os;
    }

private:
    T* _v;
};

} // namespace lcs

std::vector<std::string> split(std::string& s, const std::string& delimiter);
std::vector<std::string> split(std::string& s, const char delimiter);

/******************************************************************************
                                  LOGGING/
******************************************************************************/

#include <fstream>
extern std::optional<std::ofstream> TESTLOG;
/** Returns whether the logging target supports colors or not. */
bool is_color_enabled(void);

/** Runs an assertion, displays an error message on failure. Intended for
 * macros. */
int __expect(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept;
/** Generates the logger prefix, Intended for macros. */
std::ostream& _log_pre(std::ostream& stream, const char* status,
    const char* file_name, int line, const char* function);
/** Generates the logger prefix for tests, Intended for macros. */
std::ostream& _log_pre_f(
    const char* status, const char* file_name, int line, const char* function);

#define F_BOLD "\033[1m"
#define F_UNDERLINE "\033[4m"
#define F_RED "\033[31m"
#define F_GREEN "\033[32m"
#define F_BLUE "\033[34m"
#define F_RESET "\033[0m"

#define _INFO "INFO  | "
#define _WARN "WARN  | "
#define _ERROR "ERROR | "
#define _FATAL "FATAL | "
#define __F_INFO F_GREEN _INFO F_RESET
#define __F_WARN F_GREEN _WARN F_RESET
#define __F_ERROR F_RED _ERROR F_RESET
#define __F_FATAL F_RED _FATAL F_RESET

#define __LLOG_CUSTOM__(status, stream, file, line, function, ...)             \
    ((_log_pre(stream, (__F_##status), file, line, function)                   \
         << __VA_ARGS__ << std::endl),                                         \
        (_log_pre_f((_##status), file, line, function)                         \
            << __VA_ARGS__ << std::endl))

#define __LLOG__(STATUS, _STREAM, ...)                                         \
    __LLOG_CUSTOM__(                                                           \
        STATUS, _STREAM, __FILE_NAME__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define CLASS *this << "\t"

#define L_WARN(...) __LLOG__(WARN, std::cout, __VA_ARGS__)
#define L_ERROR(...) __LLOG__(ERROR, std::cerr, __VA_ARGS__)

#ifndef NDEBUG
#define C_INFO()
#define L_INFO(...) __LLOG__(INFO, std::cout, __VA_ARGS__)
/** Runs an assertion. Displays an error message on failure. In debug builds
 * also crashes the application. */
#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (__expect([&]() mutable -> bool { return expr; }, __FUNCTION__,     \
                __FILE_NAME__, __LINE__, #expr)) {                             \
            exit(1);                                                           \
        }                                                                      \
    }
#else
#define L_INFO(...)
#define lcs_assert(expr)                                                       \
    if ((expr) == 0) {                                                         \
        L_ERROR(F_RED F_BOLD "Assertion " << #expr << " failed!" F_RESET);     \
    }
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

inline std::string strlimit(const std::string& input, size_t limit)
{
    size_t len = input.length();
    if (len > limit) { return "..." + input.substr(len - limit + 3, len); }
    return input;
}
