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

#include "core.h"

namespace lcs {
namespace parse {

    /**
     * Serializes a scene to JSON.
     * @param s - scene to clone
     * @returns JSON::Value
     */
    std::string to_json(Scene& s);

    /**
     * Reads from a JSON document and generates a scene
     * @param doc - to read from
     * @param s - to save into
     * @returns error on failure
     */
    error_t from_json(const std::string& doc, Scene& s);

} // namespace parse
} // namespace lcs
