#pragma once
/*******************************************************************************
 * \file
 * File: include/port.h
 * Created: 07/31/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/
#include "common.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
namespace Json {
class Value;
}
namespace lcs {
/** Initializes required libraries. */
namespace net {
    void init(void);

    /** Send a GET request to targeted URL.
     * @param URL target URL.
     * @param resp response to update.
     * @param authorization header, optional.
     * @returns Error on failure:
     *
     *  - Error::REQUEST_FAILED
     *  - Error::RESPONSE_ERROR
     */
    LCS_ERROR get_request(const std::string& URL, std::string& resp,
        const std::string& authorization = "");

    /** Intended for binary data. See lcs::net::get_request. */
    LCS_ERROR get_request(const std::string& url,
        std::vector<unsigned char>& resp,
        const std::string& authorization = "");

    /** Send a POST request to targeted URL.
     * @param URL target URL.
     * @param resp response to update.
     * @param req request body, optional.
     * @param authorization header, optional.
     * @returns Error on failure:
     *
     *  - Error::REQUEST_FAILED
     *  - Error::RESPONSE_ERROR
     */
    LCS_ERROR post_request(const std::string& URL, std::string& resp,
        const std::string& req = "", const std::string& authorization = "");

    inline uint32_t htonl(uint32_t host)
    {
        uint16_t test = 1;
        if (*(reinterpret_cast<uint8_t*>(&test)) == 1) {
            uint32_t network = 0;
            network |= ((host >> 24) & 0xFF);
            network |= ((host >> 8) & 0xFF00);
            network |= ((host << 8) & 0xFF0000);
            network |= ((host << 24) & 0xFF000000);
            return network;
        }
        return host;
    }

    inline uint32_t ntohl(uint32_t network) { return htonl(network); }
} // namespace net

/******************************************************************************
                                  LOGGING/
******************************************************************************/

extern std::ofstream __TEST_LOG__;
enum LogLevel { DEBUG, INFO, WARN, ERROR };
static constexpr const char* LogLevel_str(LogLevel l)
{
    switch (l) {
    case DEBUG: return "DEBUG";
    case INFO: return "INFO ";
    case WARN: return "WARN ";
    default: return "ERROR";
    }
}

struct Line {
    Line()            = default;
    LogLevel severity = DEBUG;
    std::array<char, 12> time_str {};
    std::array<char, 6> log_level_str {};
    std::array<char, 18> obj {};
    std::array<char, 15> file_line {};
    std::array<char, 25> fn {};
    std::array<char, 512> expr {};

    template <typename... Args>
    Line(LogLevel l, const char* file, int line, const char* _fn,
        const char* fmt, Args... args)
    {
        _set_time();
        severity = l;
        std::strncpy(log_level_str.data(), LogLevel_str(l),
            log_level_str.max_size() - 1);
        std::snprintf(
            file_line.data(), file_line.max_size() - 1, "%s:%3d", file, line);
        _fn_parse(_fn);
        std::snprintf(expr.data(), expr.max_size() - 1, fmt, args...);
    }

private:
    void _fn_parse(std::string fnname);
    void _set_time(void);
};

/**
 * Push a log message to the stack. Intended to be used by the macros such as
 * L_INFO, L_WARN, L_ERROR, L_DEBUG.
 * @param line to push
 *
 */
void l_push(Line&& line);

/** Clears all log messages. */
void l_clear(void);

/**
 * Loop over existing logs starting from the oldest.
 * @param f iteration function
 */
void l_iterate(std::function<void(size_t, const Line&)> f);

/** Runs an assertion, displays an error message on failure. Intended for
 * macros. */
int __expect(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept;

#define __LLOG__(STATUS, ...)                                                  \
    lcs::l_push(lcs::Line {                                                    \
        STATUS, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__ })

#define L_INFO(...) __LLOG__(LogLevel::INFO, __VA_ARGS__)
#define L_WARN(...) __LLOG__(LogLevel::WARN, __VA_ARGS__)
#define L_ERROR(...) __LLOG__(LogLevel::ERROR, __VA_ARGS__)

#ifndef NDEBUG
#define L_DEBUG(...) __LLOG__(LogLevel::DEBUG, __VA_ARGS__)
/** Runs an assertion. Displays an error message on failure. In debug builds
 * also crashes the application. */
#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (lcs::__expect([&]() mutable -> bool { return expr; },              \
                __FUNCTION__, __FILE_NAME__, __LINE__, #expr)) {               \
            exit(1);                                                           \
        }                                                                      \
    }
#else
#define L_DEBUG(...)
#define lcs_assert(expr)                                                       \
    {                                                                          \
        lcs::__expect([&]() mutable -> bool { return expr; }, __FUNCTION__,    \
            __FILE_NAME__, __LINE__, #expr);                                   \
    }
#endif

#define S_ERROR(msg, ...) (L_ERROR(msg)), __VA_ARGS__
#define ERROR(err) (L_ERROR("%s: %s", #err, errmsg(err))), err
#ifndef WARN
#define WARN(err) (L_WARN("%s: %s", #err, errmsg(err))), err
#endif
#define VEC_TO_STR(os, vec, ...)                                               \
    for (const auto& iter : vec) {                                             \
        os << __VA_ARGS__(iter) << ",\t";                                      \
    }

/******************************************************************************
                                  /LOGGING
******************************************************************************/

/******************************************************************************
                                    IO/
******************************************************************************/

extern std::filesystem::path ROOT;
extern std::filesystem::path TMP;

extern std::filesystem::path LIBRARY;
extern std::filesystem::path LOCAL;
extern std::filesystem::path CACHE;
extern std::filesystem::path MISC;
extern std::string INI;
extern bool is_testing;

/**
 * Initializes required folder structure for the application.
 * If testing flag is enabled files are saved to a temporary location.
 * @param is_testing whether testing mode is enabled or not
 */
void init_paths(bool is_testing = false);

/**
 * Reads contents of the given string file and writes it to the buffer.
 * @param path to read
 * @param data to save
 * @returns whether reading is successful or not
 */
bool read(const std::string& path, std::string&);

/**
 * Reads contents of the given binary file and writes it to the buffer.
 * @param path to read
 * @param data to save
 * @returns whether reading is successful or not
 */
bool read(const std::string& path, std::vector<unsigned char>&);

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

template <typename T> const char* to_str(T);
} // namespace lcs
