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

#include "errors.h"
#include <libintl.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#define _(String) gettext(String)

#define VERSION 1

#define APP_PKG "com.lcs.app"
#define APPNAME "LCS"
#define APPNAME_BIN "LogicCircuitSimulator"
#define APPNAME_LONG "Logic Circuit Simulator"
extern const char* __LICENSE__;

#ifndef API_ENDPOINT
#ifndef NDEBUG
#define API_ENDPOINT "http://localhost:8000"
#else
#define API_ENDPOINT "https://lcs2.com"
#endif
#endif
#define LCS_ERROR [[nodiscard("Error codes must be handled")]] Error

namespace lcs {
template <typename T> const char* to_str(T);
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
    inline T& operator*() { return *_v; }
    inline const T& operator*() const { return *_v; }

private:
    T* _v;
};

/******************************************************************************
                                  LOGGING/
******************************************************************************/

/**
 * Single Log message. Log messages are printed to the terminal,
 * written to the test output file and displayed on the Console window.
 */
struct Message {
    enum Severity { DEBUG, INFO, WARN, ERROR, FATAL };
    Message()         = default;
    Severity severity = DEBUG;
    std::array<char, 12> time_str {};
    std::array<char, 6> log_level_str {};
    std::array<char, 18> obj {};
    std::array<char, 20> file_line {};
    std::array<char, 25> fn {};
    std::array<char, 512> expr {};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
    template <typename... Args>
    Message(Severity l, const char* file, int line, const char* _fn,
        const char* fmt, Args... args)
    {
        _set_time();
        severity = l;
        std::strncpy(
            log_level_str.data(), _severity_to_str(l), log_level_str.size());
        std::snprintf(
            file_line.data(), file_line.max_size(), "%s:%-4d", file, line);
        _fn_parse(_fn);
        std::snprintf(expr.data(), expr.max_size(), fmt, args...);
    }
#pragma GCC diagnostic pop

private:
    void _fn_parse(const char* name);
    void _set_time(void);
    static constexpr const char* _severity_to_str(Severity l)
    {
        switch (l) {
        case FATAL: return _("FATAL");
        case DEBUG: return _("DEBUG");
        case INFO: return _("INFO ");
        case WARN: return _("WARN ");
        default: return _("ERROR");
        }
    }
};

namespace fs {

#define __LLOG__(STATUS, ...)                                                  \
    lcs::fs::_log(Message {                                                    \
        STATUS, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__ })

#define L_INFO(...) __LLOG__(Message::INFO, __VA_ARGS__)
#define L_WARN(...) __LLOG__(Message::WARN, __VA_ARGS__)
#define L_ERROR(...) __LLOG__(Message::ERROR, __VA_ARGS__)
/** Runs an assertion. Displays an error message on failure. In debug builds
 * also crashes the application. */
#define lcs_assert(expr)                                                       \
    {                                                                          \
        try {                                                                  \
            if (!(expr)) {                                                     \
                __LLOG__(Message::FATAL, "Assertion \"" #expr "\" failed!");   \
                exit(1);                                                       \
            }                                                                  \
        } catch (const std::exception& ex) {                                   \
            __LLOG__(Message::FATAL,                                           \
                " Assertion \"" #expr                                          \
                "\" failed with an exception! Cause: %s",                      \
                ex.what());                                                    \
            exit(1);                                                           \
        } catch (const std::string& ex) {                                      \
            __LLOG__(Message::FATAL,                                           \
                "Assertion \"" #expr "\" failed with an exception! Cause: %s", \
                ex.c_str());                                                   \
            exit(1);                                                           \
        }                                                                      \
    }

#ifndef NDEBUG
#define L_DEBUG(...) __LLOG__(Message::DEBUG, __VA_ARGS__)

#else
#define L_DEBUG(...)
#endif

#define S_ERROR(msg, ...) (L_ERROR(msg)), __VA_ARGS__
#define ERROR(err) ((L_ERROR("%s(%d): %s", #err, err, errmsg(err))), err)
#define VEC_TO_STR(os, vec, ...)                                               \
    for (const auto& iter : vec) {                                             \
        os << __VA_ARGS__(iter) << ",\t";                                      \
    }

    /**
     * Initializes required folder structure for the application.
     * If testing flag is enabled files are saved to a temporary location.
     * @param is_testing whether testing mode is enabled or not
     */
    void init(bool is_testing = false);
    void close(void);

    /**
     * Loop over existing logs starting from the oldest.
     * @param fn iteration function
     */
    void logs_for_each(std::function<void(size_t, const Message& l)> fn);

    /** Clears all log messages. */
    void clear_log(void);

    extern bool is_testing;
    extern std::filesystem::path ROOT;
    extern std::filesystem::path TMP;
    extern std::filesystem::path LIBRARY;
    extern std::filesystem::path CACHE;
    extern std::filesystem::path MISC;
    extern std::filesystem::path LOCALE;
    extern std::vector<std::string> LOCALE_LANG;
    extern std::string INI;
    extern FILE* __TEST_LOG__;

    /** Runs an assertion, displays an error message on failure. Intended
     * for macros. */
    int __expect(std::function<bool(void)> expr, const char* function,
        const char* file, int line, const char* str_expr) noexcept;
    /**
     * Push a log message to the stack. Intended to be used by the macros
     * such as L_INFO, L_WARN, L_ERROR, L_DEBUG.
     */
    void _log(const Message& l);

    /**
     * Reads contents of the given string file and writes it to the buffer.
     * @param path to read
     * @param data to save
     * @returns whether reading is successful or not
     */
    bool read(const std::string& path, std::string& data);

    /**
     * Reads contents of the given binary file and writes it to the buffer.
     * @param path to read
     * @param data to save
     * @returns whether reading is successful or not
     */
    bool read(const std::string& path, std::vector<unsigned char>& data);

    /**
     * Write contents of data to the desired path.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(const std::string& path, const std::string& data);

    /**
     * Write contents of data to the desired path. Used for binary files.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(const std::string& path, std::vector<unsigned char>& data);

} // namespace fs

/******************************************************************************
                                  /LOGGING
******************************************************************************/

} // namespace lcs

template <typename T, typename... Args> T get_first(T first, Args...)
{
    return first;
}

std::string strlimit(const std::string& input, size_t limit);
std::vector<std::string> split(std::string& s, const std::string& delimiter);
std::vector<std::string> split(std::string& s, const char delimiter);
std::string base64_encode(const std::string& input);
std::string base64_decode(const std::string& input);

/** Converts the unsigned integer hostlong from host byte order to network
 * byte order.
 * @param host number to convert
 * * Conforming to POSIX.1-2001.
 */
inline uint32_t htonl(uint32_t host)
{
    uint32_t test = 1;
    if (*(uint8_t*)&test == 1) {
        uint32_t network = 0;
        network |= ((host >> 24) & 0xFF);
        network |= ((host >> 8) & 0xFF00);
        network |= ((host << 8) & 0xFF0000);
        network |= ((host << 24) & 0xFF000000);
        return network;
    }
    return host;
}

/**  Converts the unsigned integer netlong from network byte order to host byte
 * order.
 * @param network number to convert
 * * Conforming to POSIX.1-2001.
 */
inline uint32_t ntohl(uint32_t network) { return htonl(network); }
