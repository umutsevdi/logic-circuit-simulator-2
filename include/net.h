#pragma once
/*******************************************************************************
 * \file
 * File: include/net.h
 * Created: 06/13/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/
#include <string>
namespace lcs::net {

enum Error {
    SUCCESS = 0,
    CURL_INIT_FAILED,
    CURL_REQUEST_FAILED,
    CURL_RESPONSE_ERROR,
    // Add more error codes as needed
};

/** Initializes required libraries. */
void init(void);

struct Packet {
    std::string url;
    /** Bearer session token */
    std::string session_token;
};

/** Send a get request to targeted URL.
 * @param pkt Required information to perform request.
 * @err error code to update on failure.
 *
 * @returns body
 */
Error get(const Packet& pkt, std::string& resp);
/** Send a post request to targeted URL.
 * @param pkt Required information to perform request.
 * @param req request body.
 * @err error code to update on failure.
 *
 * @returns body
 */
Error post(const Packet& pkt, const std::string& req, std::string& resp);

struct DeviceFlow {
    enum PollState { IDLE, POLLING, BROKEN, TIMEOUT, DONE };
    std::string client_id;
    std::string device_code;
    std::string user_code;
    std::string verification_uri;
    int expires_in;
    int interval;

    PollState poll(void);

    time_t start_time;

    time_t _last_poll;
};
Error start_device_flow(DeviceFlow&);

struct Account {
    std::string login;
    std::string name;
    std::string email;
    std::string bio;
    std::string url;
    std::string avatar_url;
};
bool is_logged_in(void);
const Account* get_account(void);
void open_browser(const std::string& url);

} // namespace lcs::net
