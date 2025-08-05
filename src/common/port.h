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

#include <errors.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Json {
class Value;
}
namespace lcs {
namespace net {
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

    /** Send a POST request to targeted URL.
     * @param URL target URL.
     * @param req request body.
     * @param authorization header, optional.
     *
     * @returns HttpRequestId
     */
    uint64_t post_request(const std::string& URL,
        const std::vector<uint8_t>& req, const std::string& authorization = "");

    /**
     * Attempts to obtain a completed request, updating session on success.
     * @param id to query.
     * @param session to update
     *
     * @returns whether request is completed or not.
     */
    bool pull_response(uint64_t id, HttpResponse& session);
} // namespace net

/******************************************************************************
                                    IO/
******************************************************************************/

/**
 * Reads contents of the given string file and writes it to the buffer.
 * @param path to read
 * @param data to save
 * @returns whether reading is successful or not
 */
bool read(const std::string& path, std::string& data);

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

void request_open_file(void);
void request_save_file(const char* path);
void request_alert(const char* msg);
void request_close(void);
void handle_requests(void);

} // namespace lcs
