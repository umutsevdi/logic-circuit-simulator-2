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

#include <cstdint>
#include <errors.h>
#include <functional>
#include <string>
#include <vector>

namespace lcs ::net {
void init(bool testing = false);
void close(void);

struct HttpResponse {
    std::vector<uint8_t> data;
    int status_code = 0;
    Error err       = Error::OK;
};

/** Send a GET request to targeted URL.
 * @param URL target URL.
 * @param authorization header, optional.
 *
 * @returns HttpRequestId
 */
uint64_t get_request(
    const std::string& URL, const std::string& authorization = "");

/** Send a GET request to targeted URL. Execute a callback upon receiving
 * response.
 * @param cb callback function.
 * @param URL target URL.
 * @param authorization header, optional.
 */
void get_request_then(std::function<void(HttpResponse& resp)> cb,
    const std::string& URL, const std::string& authorization = "");

/** Send a POST request to targeted URL.
 * @param URL target URL.
 * @param req request body.
 * @param authorization header, optional.
 *
 * @returns HttpRequestId
 */
uint64_t post_request(const std::string& URL, const std::vector<uint8_t>& req,
    const std::string& authorization = "");

/** Send a POST request to targeted URL. Execute a callback upon receiving
 * response.
 * @param cb callback function.
 * @param URL target URL.
 * @param req request body.
 * @param authorization header, optional.
 */
void post_request_then(std::function<void(HttpResponse& resp)> cb,
    const std::string& URL, const std::vector<uint8_t>& req,
    const std::string& authorization = "");

/**
 * Attempts to obtain a completed request, updating session on success.
 * @param id to query.
 * @param session to update
 *
 * @returns whether request is completed or not.
 */
bool pull_response(uint64_t id, HttpResponse& session);

} // namespace lcs::net

void open_browser(const std::string& url);
