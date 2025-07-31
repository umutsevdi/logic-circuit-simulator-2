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

/** Device flow authenticates the device with non-blocking mechanism. */
class AuthenticationFlow {

public:
    enum State { INACTIVE, STARTED, POLLING, DONE, TIMEOUT, BROKEN };
    AuthenticationFlow()  = default;
    ~AuthenticationFlow() = default;
    std::string client_id;
    std::string device_code;
    std::string user_code;
    std::string verification_uri;
    int expires_in    = 0;
    int interval      = 0;
    time_t start_time = 0L;

    Error start(void);
    State poll(void);
    State get_state(void) const;
    void resolve(void);
    const char* reason(void) const;

    /** Run authentication flow during initialization. */
    Error start_existing(void);

private:
    time_t _last_poll  = 0L;
    State _last_status = INACTIVE;
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
inline bool is_logged_in(void)
{
    return get_flow().get_state() == AuthenticationFlow::DONE;
}

/**
 * Opens up the given URL on the user's default web browser.
 * @param url to open
 */
void open_browser(const std::string& url);

LCS_ERROR upload_scene(NRef<const Scene> scene, std::string resp);

} // namespace lcs::net
