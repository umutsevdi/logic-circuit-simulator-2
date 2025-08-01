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

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#define VERSION 1

#define APP_PKG "com.lcs.app"
#define APPNAME "LCS"
#define APPNAME_LONG "LogicCircuitSimulator"

#ifndef API_ENDPOINT
#ifndef NDEBUG
#define API_ENDPOINT "http://localhost:8000"
#else
#define API_ENDPOINT "https://lcs2.com"
#endif
#endif

namespace lcs {
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

template <typename T> const char* to_str(T);

std::string base64_encode(const std::string& input);
std::string base64_decode(const std::string& input);

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

enum Error {
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
    /** Only components can have NodeType::COMPONENT_INPUT and
       NodeType::COMPONENT_OUTPUT. */
    NOT_A_COMPONENT,
    /** Inputs can not be used as a to type. */
    INVALID_TO_TYPE,
    /** Attempted to bind a already connected input socket. */
    ALREADY_CONNECTED,
    /** Component socket is not connected. */
    NOT_CONNECTED,
    /** Attempted to execute or load a component that does not exist. */
    COMPONENT_NOT_FOUND,
    /** File is not a valid scene. */
    INVALID_MGC,
    /** Deserialized node does not fulfill its requirements. */
    INVALID_NODE,
    /** Deserialized input does not fulfill its requirements. */
    INVALID_INPUT,
    /** Deserialized gate does not fulfill its requirements. */
    INVALID_GATE,
    /** Deserialized scene does not fulfill its requirements. */
    INVALID_SCENE,
    /** Deserialized scene name exceeds the character limits. */
    INVALID_SCENE_NAME,
    /** Deserialized name of the scene's author exceeds the character limits. */
    INVALID_AUTHOR_NAME,
    /** Deserialized description of the scene exceeds the character limits. */
    INVALID_DESCRIPTION,
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
    /** Failed to send the request to the server. Request didn't arrive to the
       server. */
    REQUEST_FAILED,
    /** Request was sent successfully to the server but the server responded
     * with non 200 code message.*/
    RESPONSE_ERROR,
    /** Response is not a valid JSON string. */
    JSON_PARSE_ERROR,

    /** See error message.*/
    KEYCHAIN_GENERIC_ERROR,
    /** Key not found.*/
    KEYCHAIN_NOT_FOUND,
    /** Occurs on Windows when the number of characters in password is too
       long.*/
    KEYCHAIN_TOO_LONG,
    /** Occurs on MacOS when the application fails to pass certain conditions.*/
    KEYCHAIN_ACCESS_DENIED,
    /** Attempted to start a flow that was already active. */
    UNTERMINATED_FLOW,
    /** Couldn't complete the flow in expected duration. */
    FLOW_TIMEOUT,
    /** Flow failed. */
    FLOW_FAILURE,
    /** Represents the how many types of error codes exists. Not a valid error
       code.*/
    ERROR_S
};

constexpr const char* errmsg(Error e)
{
    switch (e) {
    case OK: return "Operation is successful.";
    case INVALID_NODEID: return "Object has invalid ID.";
    case INVALID_RELID: return "Object has invalid relaitonship ID.";
    case REL_NOT_FOUND: return "The relationship was not found.";
    case NODE_NOT_FOUND: return "The nodewas not found.";
    case INVALID_FROM_TYPE: return "Outputs can not be used as a from type.";
    case NOT_A_COMPONENT: return "Only components can have CIN or COUT.";
    case INVALID_TO_TYPE: return "Inputs can not be used as a to type.";
    case ALREADY_CONNECTED: return "Input socket is already connected.";
    case NOT_CONNECTED: return "Component socket is not connected. ";
    case COMPONENT_NOT_FOUND: return "Component was not found.";
    case INVALID_MGC: return "Invalid file format.";
    case INVALID_NODE: return "Invalid node format.";
    case INVALID_INPUT: return "Invalid InputNode format.";
    case INVALID_GATE: return "Invalid GateNode format.";
    case INVALID_SCENE: return "Invalid scene format. ";
    case INVALID_SCENE_NAME: return "Scene name is too long.";
    case INVALID_AUTHOR_NAME: return "Author name is too long.";
    case INVALID_DESCRIPTION: return "Description is too long.";
    case INVALID_COMPONENT: return "Invalid component.";
    case REL_CONNECT_ERROR: return "An error occurred while connecting a node.";
    case INVALID_DEPENDENCY_FORMAT: return "Invalid dependency string. ";
    case UNDEFINED_DEPENDENCY: return "Undefined dependency.";
    case INVALID_JSON_FORMAT: return "Invalid JSON document.";
    case NOT_A_JSON: return "Invalid file format.";
    case NOT_FOUND: return "No such file or directory.";
    case NO_SAVE_PATH_DEFINED: return "Failed to save file.";
    case REQUEST_FAILED: return "Failed to send the request.";
    case RESPONSE_ERROR: return "Bad request.";
    case JSON_PARSE_ERROR: return "Response is not a valid JSON string.";
    case KEYCHAIN_GENERIC_ERROR: return "Keychain couldn't load the password.";
    case KEYCHAIN_NOT_FOUND: return "Key was not found.";
    case KEYCHAIN_TOO_LONG: return "[WindowsOnly] key is too long.";
    case KEYCHAIN_ACCESS_DENIED: return "[AppleOnly] Authorization failure.";
    case UNTERMINATED_FLOW: return "Already active flowl.";
    case FLOW_TIMEOUT: return "The flow exceeded the expected time limit.";
    case FLOW_FAILURE: return "The flow encountered an error.";
    default: return "Unknown Error.";
    }
};
#define LCS_ERROR [[nodiscard("Error codes must be handled")]] Error
} // namespace lcs
