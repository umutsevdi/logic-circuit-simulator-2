#pragma once
/*******************************************************************************
 * \file
 * File: /parse/parse.h
 * Created: 04/11/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include <json/json.h>

namespace lcs {
namespace parse {

    enum error_t {
        OK,
        INVALID_NODE,
        INVALID_INPUT,
        INVALID_GATE,
        INVALID_SCENE,
        PARSE_ERROR,
        NODE_NOT_FOUND,
        REL_CONNECT_ERROR,
        INVALID_CONTEXT,

        WRITE_FMT
    };

    /**
     * Serializes a scene to JSON.
     * @param s - scene to clone
     * @returns JSON::Value
     */
    Json::Value to_json(Scene& s);

    /**
     * Reads from a JSON document and generates a scene
     * @param doc - to read from
     * @param s - to save into
     * @returns error on failure
     */
    error_t from_json(const Json::Value& doc, Scene& s);

} // namespace parse
} // namespace lcs
