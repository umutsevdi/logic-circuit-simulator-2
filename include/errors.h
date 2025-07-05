#pragma once
/*******************************************************************************
 * \file
 * File: include/errors.h
 * Created: 07/05/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

namespace lcs {
#define LCS_ERROR [[nodiscard("Error codes must be handled")]] Error
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

    /** Represents the how many types of error codes exists. Not a valid error
       code.*/
    ERROR_S
};
} // namespace lcs
