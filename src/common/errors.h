#pragma once
/*******************************************************************************
 * \file
 * File: errors.h
 * Created: 02/04/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

namespace lcs {
enum Error {
    /** Operation is successful. */
    OK,
    /** Parser encountered end of file before end of a instruction. */
    INCOMPLETE_INSTR,
    /** Deserialization failed due to unexpected byte in the file. */
    INVALID_BYTE,
    /** Parser encountered a invalid string. */
    INVALID_STRING,
    /** Object has 0 as node id. */
    INVALID_NODEID,
    /** Object has 0 as rel id. */
    INVALID_RELID,
    /** Given relationship does not exist within that scene. */
    REL_NOT_FOUND,
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
    /** Deserialized node does not fulfill its requirements. */
    INVALID_NODE,
    /** Invalid dependency string. */
    INVALID_DEPENDENCY_FORMAT,
    /** Not a valid Scene document. */
    INVALID_SCENE_FORMAT,
    /** Invalid JSON format. */
    INVALID_JSON,
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
    /** Native File Dialog related error. */
    NFD,
    /** Error while updating the locale. */
    LOCALE_ERROR,
    /** Represents the how many types of error codes exists. Not a valid error
       code.*/
    ERROR_S
};

constexpr const char* errmsg(Error e)
{
    switch (e) {
    case OK: return "Operation is successful.";
    case INCOMPLETE_INSTR:
        return "Parser encountered end of file before end of a instruction.";
    case INVALID_BYTE:
        return "Deserialization failed due to unexpected byte in the file.";
    case INVALID_STRING: return "Parser encountered a invalid string. ";
    case INVALID_NODEID: return "Object has invalid ID.";
    case INVALID_RELID: return "Object has invalid relaitonship ID.";
    case REL_NOT_FOUND: return "The relationship was not found.";
    case INVALID_FROM_TYPE: return "Outputs can not be used as a from type.";
    case NOT_A_COMPONENT: return "Only components can have CIN or COUT.";
    case INVALID_TO_TYPE: return "Inputs can not be used as a to type.";
    case ALREADY_CONNECTED: return "Input socket is already connected.";
    case NOT_CONNECTED: return "Component socket is not connected. ";
    case COMPONENT_NOT_FOUND: return "Component was not found.";
    case INVALID_NODE: return "Invalid node format.";
    case INVALID_DEPENDENCY_FORMAT: return "Invalid dependency string. ";
    case INVALID_JSON: return "Invalid JSON format.";
    case INVALID_SCENE_FORMAT: return "Invalid scene document.";
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
    case NFD: return "NFD Eror.";
    case LOCALE_ERROR: return "Error while updating the locale.";

    case ERROR_S: return "Unknown Error.";
    }
};

#ifdef ERROR
#undef ERROR
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
} // namespace lcs
