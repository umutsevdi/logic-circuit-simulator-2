#pragma once
/*******************************************************************************
 * \file
 * File: api/api.h
 * Created: 08/16/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License:
 ******************************************************************************/
#include "common.h"
#include <json/value.h>
namespace lcs::api {

namespace gh {
    LCS_ERROR get_device_code(Json::Value& response, const std::string& _id);
    LCS_ERROR get_access_token(Json::Value& response,
        const std::string& client_id, const std::string& device_code);
}; // namespace gh

namespace rest {
    LCS_ERROR get_client_id(Json::Value& response);

    LCS_ERROR login_oauth(Json::Value& response, const std::string& token = "");

    LCS_ERROR login_session(
        Json::Value& response, const std::string& token = "");

    LCS_ERROR get_user_profile(Json::Value& response);

    LCS_ERROR get_repo_list(
        Json::Value& response, const std::string& query, int page);

} // namespace rest

} // namespace lcs::api
