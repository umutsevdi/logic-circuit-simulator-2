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

#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>

#define APP_PKG "com.lcs.app"
#define APPNAME "LCS"
#define APPNAME_LONG "LogicCircuitSimulator"

#ifndef API_ENDPOINT
#ifndef NDEBUG
#define API_ENDPOINT "http://localhost:8080"
#else
#define API_ENDPOINT "https://lcs2.com"
#endif
#endif

#include "errors.h"
namespace Json {
class Value;
}
namespace lcs {
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
 * Reads contents of the given JSON file and returns it as a string.
 * @param path to read
 * @returns data
 */
std::string read(const std::string& path);

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

/******************************************************************************
                                    TYPES
******************************************************************************/

/** id type for socket, sock_t = 0 means disconnected */
typedef uint8_t sockid;

/** Relationship identifier. Id is a non-zero identifier. */
typedef uint32_t relid;

/** Type of a node in a scene. */
enum NodeType : uint8_t {
    /** Logic gate. */
    GATE,
    /** A component that has been loaded externally. */
    COMPONENT,
    /** An input node that can be switched manually or by a clock. */
    INPUT,
    /** An output node. */
    OUTPUT,
    /** Input slot of a component. NOTE: Only available in Component Scenes. */
    COMPONENT_INPUT,
    /** Output slot of a component. NOTE: Only available in Component Scenes. */
    COMPONENT_OUTPUT,

    NODE_S
};

constexpr const char* NodeType_to_str_full(NodeType s)
{
    switch (s) {
    case NodeType::GATE: return "Gate";
    case NodeType::COMPONENT: return "Component";
    case NodeType::INPUT: return "Input";
    case NodeType::OUTPUT: return "Output";
    case NodeType::COMPONENT_INPUT: return "Component Input";
    case NodeType::COMPONENT_OUTPUT: return "Component Output";
    default: return "Unknown";
    }
}
constexpr const char* NodeType_to_str(NodeType s)
{
    switch (s) {
    case NodeType::GATE: return "Gate";
    case NodeType::COMPONENT: return "Comp";
    case NodeType::INPUT: return "In";
    case NodeType::OUTPUT: return "Out";
    case NodeType::COMPONENT_INPUT: return "Cin";
    case NodeType::COMPONENT_OUTPUT: return "Cout";
    default: return "Unknown";
    }
}

class Serializable {
public:
    /**
     * Converts given object to a Json::Value
     * @returns Json document
     */
    virtual Json::Value to_json() const = 0;
    /**
     * Reads contents of the document and updates its fields.
     * @returns Error on failure:
     *
     *  - Error::INVALID_SCENE
     *  - Error::REL_CONNECT_ERROR
     *  - Error::UNDEFINED_DEPENDENCY
     *  - Error::INVALID_NODE
     *  - Error::INVALID_NODE
     *  - Error::INVALID_NODE
     *  - Error::INVALID_COMPONENT
     *  - Error::INVALID_INPUT
     *  - Error::INVALID_COMPONENT
     *  - Error::INVALID_GATE
     *  - Error::INVALID_JSON_FORMAT
     */
    LCS_ERROR virtual from_json(const Json::Value&) = 0;
};

/**
 * Node is a handler that represents the index.
 * id is a non-zero identifier. Together with the type, represents a unique
 * node.
 */
struct Node final : public Serializable {
    Node(uint16_t _id = 0, NodeType _type = GATE);
    Node(Node&&)                 = default;
    Node(const Node&)            = default;
    Node& operator=(Node&&)      = default;
    Node& operator=(const Node&) = default;

    bool operator<(const Node& n) const { return this->id < n.id; }

    inline uint32_t numeric(void) const { return id | (type << 16); }
    std::string to_str(void) const;

    /* Serializable Interface */
    Json::Value to_json() const override;
    LCS_ERROR from_json(const Json::Value&) override;

    uint16_t id : 16;
    NodeType type : 4;
};

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
    Node node         = 0;
    std::array<char, 12> time_str {};
    std::array<char, 6> log_level_str {};
    std::array<char, 18> obj {};
    std::array<char, 15> file_line {};
    std::array<char, 25> fn {};
    std::array<char, 512> expr {};

    template <typename... Args>
    Line(LogLevel l, const char* file, int line, const char* _fn,
        /*optional*/ Node object, const char* fmt, Args... args)
    {
        _set_time();
        severity = l;
        std::strncpy(log_level_str.data(), LogLevel_str(l),
            log_level_str.max_size() - 1);
        std::snprintf(
            file_line.data(), file_line.max_size() - 1, "%s:%3d", file, line);
        _fn_parse(_fn);
        if (object.id != 0 || object.type != 0) {
            std::strncpy(
                obj.data(), object.to_str().c_str(), obj.max_size() - 1);
            node = object;
        }
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

#define L_INFO(...) __LLOG__(lcs::LogLevel::INFO, 0, __VA_ARGS__)
#define L_WARN(...) __LLOG__(lcs::LogLevel::WARN, 0, __VA_ARGS__)
#define L_ERROR(...) __LLOG__(lcs::LogLevel::ERROR, 0, __VA_ARGS__)
#define C_DEBUG(...) __LLOG__(lcs::LogLevel::DEBUG, this->id(), __VA_ARGS__)

#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (lcs::__expect([&]() mutable -> bool { return expr; },              \
                __FUNCTION__, __FILE_NAME__, __LINE__, #expr)) {               \
            exit(1);                                                           \
        }                                                                      \
    }
#ifndef NDEBUG
#define L_DEBUG(...) __LLOG__(DEBUG, 0, __VA_ARGS__)
/** Runs an assertion. Displays an error message on failure. In debug builds
 * also crashes the application. */
#else
#define L_DEBUG(...)
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

/**
 * An interface for keeping state during long lasting flows for the user
 * interface.
 */
class Flow {
public:
    enum State {
        /** State is not active*/
        INACTIVE,
        /** Temporarily state: Flow has started, but it hasn't polled yet. */
        STARTED,
        /** Flow is active. */
        POLLING,
        /** Flow has been completed successfully. */
        DONE,
        /** Flow has been disrupted by an error. */
        BROKEN,
        /** Flow has not been completed in expected duration. */
        TIMEOUT
    };
    Flow(Flow&&)                 = delete;
    Flow(const Flow&)            = delete;
    Flow& operator=(Flow&&)      = delete;
    Flow& operator=(const Flow&) = delete;

    /** Starts the flow and resets it's variables.*/
    LCS_ERROR virtual start(void) = 0;
    /** Run required steps for a single frame. Return current result.*/
    State virtual poll(void) = 0;
    /** Get active state.*/
    State virtual get_state(void) const = 0;
    /** Cleans the artifacts and prepares it for the next load.*/
    void virtual resolve(void) = 0;
    /** On error's returns the reason of the error.*/
    const virtual char* reason(void) const = 0;

protected:
    Flow()          = default;
    virtual ~Flow() = default;
};

} // namespace lcs
/******************************************************************************
                                  /LOGGING
******************************************************************************/

inline std::string strlimit(const std::string& input, size_t limit)
{
    size_t len = input.length();
    if (len > limit) {
        return "..." + input.substr(len - limit + 3, limit);
    }
    return input;
}

template <typename T, typename... Args> const char* get_first(T first, Args...)
{
    return first;
}

template <typename... Args> const char* get_first(const char* first, Args...)
{
    return first;
}

std::vector<std::string> split(std::string& s, const std::string& delimiter);
std::vector<std::string> split(std::string& s, const char delimiter);
