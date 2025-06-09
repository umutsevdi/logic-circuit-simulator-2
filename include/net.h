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

#include "common.h"
namespace lcs::net {

/** Initializes required libraries. */
void init(void);

struct Packet {
    std::string url;
    /** Bearer session token */
    std::string session_token;
};

/** Send a get request to targeted URL.
 * @param pkt Required information to perform request.
 * @param resp response to update.
 * @returns Error on failure:
 *
 *  - Error::REQUEST_FAILED
 *  - Error::RESPONSE_ERROR
 */
Error get(const Packet& pkt, std::string& resp);
/** get for files*/
Error get(const Packet& pkt, std::vector<unsigned char>& resp);

/** Send a post request to targeted URL.
 * @param pkt Required information to perform request.
 * @param req request body.
 * @param resp response to update.
 * @returns Error on failure:
 *
 *  - Error::REQUEST_FAILED
 *  - Error::RESPONSE_ERROR
 */
Error post(const Packet& pkt, const std::string& req, std::string& resp);

/** Device flow authenticates the device with non-blocking mechanism. */
struct DeviceFlow {
    enum PollState { DONE, POLLING, BROKEN, TIMEOUT };

    std::string client_id;
    std::string device_code;
    std::string user_code;
    std::string verification_uri;
    int expires_in;
    int interval;
    time_t start_time;

    Error start(void);
    /** Polls GitHub for session token. */
    PollState poll(void);
    PollState is_waiting(void) const { return _last_status; }
    bool is_started(void) const { return _is_active; };

private:
    time_t _last_poll;
    bool _is_active;
    PollState _last_status;
};
Error login_existing_session(void);

struct Account {
    std::string login;
    std::string name;
    std::string email;
    std::string bio;
    std::string url;
    std::string avatar_url;
};

struct RepositoryInfo {
    std::string author_avatar;
    std::string author;
    std::string name;
    std::string description;
    int version;
};

const Account* get_account(void);
bool is_logged_in(void);
void open_browser(const std::string& url);

} // namespace lcs::net
