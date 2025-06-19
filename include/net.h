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

#include "common.h"
#include "core.h"
#include <string>

namespace lcs::net {

/** Initializes required libraries. */
void init(void);

/** Send a GET request to targeted URL.
 * @param url target URL.
 * @param resp response to update.
 * @param authorization header, optional.
 * @returns Error on failure:
 *
 *  - Error::REQUEST_FAILED
 *  - Error::RESPONSE_ERROR
 */
Error get_request(const std::string& url, std::string& resp,
    const std::string& authorization = "");

/** Send a GET request to desired URL. Intended for binary data.
 * See net::get_request for more details. */
Error get_request(const std::string& url, std::vector<unsigned char>& resp,
    const std::string& authorization = "");

/** Send a POST request to targeted URL.
 * @param url target URL.
 * @param resp response to update.
 * @param req request body, optional.
 * @param authorization header, optional.
 * @returns Error on failure:
 *
 *  - Error::REQUEST_FAILED
 *  - Error::RESPONSE_ERROR
 */
Error post_request(const std::string& url, std::string& resp,
    const std::string& req = "", const std::string& authorization = "");

/** Device flow authenticates the device with non-blocking mechanism. */
class AuthenticationFlow final : public Flow {
public:
    explicit AuthenticationFlow() { };
    ~AuthenticationFlow() { };
    std::string client_id;
    std::string device_code;
    std::string user_code;
    std::string verification_uri;
    int expires_in;
    int interval;
    time_t start_time;

    Error start(void) override;
    State poll(void) override;
    State get_state(void) const override;
    void resolve(void) override;
    const char* reason(void) const override;

    /** Run authentication flow during initialization. */
    Error start_existing(void);

private:
    time_t _last_poll;
    State _last_status;
    std::string _reason;
};

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
AuthenticationFlow& get_flow(void);

/** Returns the active account. */
const Account& get_account(void);

/** Returns whether the user is logged in.*/
inline bool is_logged_in(void) { return get_flow().get_state() == Flow::DONE; }

/**
 * Opens up the given URL on the user's default web browser.
 * @param url to open
 */
void open_browser(const std::string& url);

Error upload_scene(NRef<const Scene> scene, std::string resp);
} // namespace lcs::net
