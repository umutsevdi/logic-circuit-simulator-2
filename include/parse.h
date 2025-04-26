#pragma once
/*******************************************************************************
 * \file
 * File: parse.h
 * Created: 04/11/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "common.h"

namespace Json {
class Value;
}
namespace lcs {
class Scene;
namespace parse {
    class Serializable {
        /**
         * Converts given object to a Json::Value
         * @returns Json document
         */
        virtual Json::Value to_json(void) const = 0;
        /**
         * Reads contents of the document and updates its fields.
         * @returns error on failure
         */
        virtual error_t from_json(const Json::Value&) = 0;
    };

    /**
     * Reads scene data from given string and updates the given scene.
     * @param str to read from
     * @param scene to write into
     * @returns error on failure
     *
     */
    error_t load_scene(const std::string& str, Scene& scene);
} // namespace parse
} // namespace lcs
