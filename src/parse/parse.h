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

#include "core/engine.h"
#include <json/json.h>

namespace lcs {
namespace parse {

    enum error_t {
        OK,
        INVALID_NODE,
        INVALID_SCENE,
        PARSE_ERROR,
        NODE_NOT_FOUND,
        REL_CONNECT_ERROR,
        INVALID_CONTEXT,

        WRITE_FMT
    };

    Json::Value to_json(const node& v);
    Json::Value to_json(const point_t& v);
    Json::Value to_json(const Rel& v);
    Json::Value to_json(const Gate& v);
    Json::Value to_json(const Input& v);
    Json::Value to_json(const Output& v);
    Json::Value to_json(const Metadata& v);
    Json::Value to_json(Scene& s);

    error_t from_json(const Json::Value&, Scene&);

} // namespace parse
} // namespace lcs
