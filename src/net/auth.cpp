#include "base64.h"
#include "common.h"
#include "io.h"
#include "net.h"
#include "json/reader.h"
#include <ctime>
#include <filesystem>
#include <keychain/keychain.h>

namespace lcs::net {

Account account;
std::string access_token = "";
bool _is_logged_in       = false;

static Error get_client_id(std::string& id)
{
    std::string resp;
    Error err = get(Packet { API_ENDPOINT "/api/client_id", "" }, resp);
    if (err) {
        L_INFO(resp);
        return err;
    }
    L_INFO("Received " << resp);
    Json::Value v;
    Json::Reader r;
    if (!r.parse(resp, v)) {
        return ERROR(Error::JSON_PARSE_ERROR);
    }
    id = v["id"].asString();
    return Error::OK;
}

Error DeviceFlow::start(void)
{
    L_INFO("Calling flow");
    std::string _id;
    Error err = get_client_id(_id);
    if (err) {
        return err;
    }
    std::string body;
    err = net::post(Packet { "https://github.com/login/device/code?client_id="
                            + _id + "&scope=read:user",
                        "" },
        "", body);

    if (err) {
        return err;
    }
    Json::Value v;
    Json::Reader r;
    if (!r.parse(body, v)) {
        return ERROR(Error::JSON_PARSE_ERROR);
    }
    L_INFO("Received: " << v.toStyledString());
    client_id        = _id;
    device_code      = v["device_code"].asString();
    user_code        = v["user_code"].asString();
    verification_uri = v["verification_uri"].asString();
    expires_in       = v["expires_in"].asInt();
    interval         = v["interval"].asInt();
    start_time       = time(0);
    _last_poll       = start_time;
    _is_active       = true;

    L_INFO("Visit: https://github.com/login/device");
    L_INFO("Enter the code: " << v["user_code"].asString());
    open_browser(verification_uri);
    return Error::OK;
}

DeviceFlow::PollState DeviceFlow::poll(void)
{
    time_t now = time(nullptr);
    if (difftime(now, this->start_time) > this->expires_in) {
        _last_status = DeviceFlow::PollState::TIMEOUT;
        return ERROR(DeviceFlow::PollState::TIMEOUT);
    }
    if (difftime(now, this->_last_poll) < this->interval * 1.5) {
        _last_status = DeviceFlow::PollState::POLLING;
        return DeviceFlow::PollState::POLLING;
    }
    this->_last_poll = now;

    std::string gh_access_token;
    {
        L_INFO("Get oauth access token");
        Json::Value req;
        req["client_id"]   = client_id;
        req["device_code"] = device_code;
        req["grant_type"]  = "urn:ietf:params:oauth:grant-type:device_code";

        L_INFO("Sending:" << req.toStyledString());
        Error err
            = post(Packet { "https://github.com/login/oauth/access_token", "" },
                req.toStyledString(), gh_access_token);
        if (err) {
            _last_status = DeviceFlow::PollState::BROKEN;
            _is_active   = false;
            L_WARN("Received " << gh_access_token);
            return ERROR(DeviceFlow::PollState::BROKEN);
        }

        Json::Value resp_root;
        Json::Reader resp {};
        if (!resp.parse(gh_access_token, resp_root)) {
            _last_status = DeviceFlow::PollState::BROKEN;
            _is_active   = false;
            return ERROR(DeviceFlow::PollState::BROKEN);
        }
        if (resp_root["error"].isString()
            && resp_root["error"].asString() == "authorization_pending") {
            // TODO For other types of error codes change the to
            // DeviceFlow::PollState::BROKEN
            _last_status = DeviceFlow::PollState::POLLING;
            return DeviceFlow::PollState::POLLING;
        }
        L_INFO("Received " << resp_root.toStyledString());
        access_token = resp_root["access_token"].asString();
    }

    {
        L_INFO("Send token to session server");
        std::string session_resp;
        Error err = get(
            Packet { API_ENDPOINT "/api/login", access_token }, session_resp);
        if (err) {
            _last_status = DeviceFlow::PollState::BROKEN;
            _is_active   = false;
            L_WARN("Received " << session_resp);
            return ERROR(DeviceFlow::PollState::BROKEN);
        }
        Json::Value resp_root;
        Json::Reader resp {};
        if (!resp.parse(session_resp, resp_root)) {
            _last_status = DeviceFlow::PollState::BROKEN;
            _is_active   = false;
            return ERROR(DeviceFlow::PollState::BROKEN);
        }
        L_INFO("Received: " << resp_root.toStyledString());
        if (!resp_root["login"].isString()) {
            _last_status = DeviceFlow::PollState::BROKEN;
            _is_active   = false;
            return ERROR(DeviceFlow::PollState::BROKEN);
        }

        account.login      = resp_root["login"].asString();
        account.name       = resp_root["name"].asString();
        account.email      = resp_root["email"].asString();
        account.bio        = resp_root["bio"].asString();
        account.url        = resp_root["url"].asString();
        account.avatar_url = resp_root["avatarUrl"].asString();
        keychain::Error keyerr;
        keychain::setPassword(
            APP_PKG, APPNAME_LONG, account.login, access_token, keyerr);
        if (keyerr.type != keychain::ErrorType::NoError) {
            L_WARN(keyerr.message);
        }
        io::write(io::ROOT / ".login", account.login);

        if (!std::filesystem::exists(
                io::CACHE / (base64_encode(account.avatar_url) + ".jpeg"))) {
            std::vector<unsigned char> data;
            net::get(Packet { account.avatar_url, "" }, data);
            if (!data.empty()) {
                io::write(
                    io::CACHE / (base64_encode(account.avatar_url) + ".jpeg"),
                    data);
            }
        }
    }

    _is_logged_in = true;
    _is_active    = false;
    _last_status  = DeviceFlow::PollState::DONE;
    return DeviceFlow::PollState::DONE;
}

bool is_logged_in(void) { return _is_logged_in; }
const Account* get_account(void)
{
    if (_is_logged_in) {
        return &account;
    };
    return nullptr;
}

Error login_existing_session(void)
{
    std::string login = io::read(io::ROOT / ".login");
    if (login == "") {
        return ERROR(Error::KEYCHAIN_NOT_FOUND);
    }
    keychain::Error keyerr;
    std::string pwd
        = keychain::getPassword(APP_PKG, APPNAME_LONG, login, keyerr);
    if (keyerr.type != keychain::ErrorType::NoError) {
        L_WARN(keyerr.message);
        return (Error)(keyerr + KEYCHAIN_GENERIC_ERROR - 1);
    }

    std::string resp_str;
    Error err = get(Packet { API_ENDPOINT "/api/login", pwd }, resp_str);
    if (err) {
        return err;
    }
    Json::Value resp_root;
    Json::Reader resp {};
    if (!resp.parse(resp_str, resp_root)) {
        return ERROR(Error::JSON_PARSE_ERROR);
    }
    L_INFO("Received: " << resp_root.toStyledString());
    access_token       = pwd;
    account.login      = resp_root["login"].asString();
    account.name       = resp_root["name"].asString();
    account.email      = resp_root["email"].asString();
    account.bio        = resp_root["bio"].asString();
    account.url        = resp_root["url"].asString();
    account.avatar_url = resp_root["avatarUrl"].asString();
    _is_logged_in      = true;

    if (!std::filesystem::exists(
            io::CACHE / (base64_encode(account.avatar_url) + ".jpeg"))) {
        std::vector<unsigned char> data;
        net::get(Packet { account.avatar_url, "" }, data);
        if (!data.empty()) {
            io::write(io::CACHE / (base64_encode(account.avatar_url) + ".jpeg"),
                data);
        }
    }
    return Error::OK;
}

void open_browser(const std::string& url)
{
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#else
    std::string command = "xdg-open " + url; // For Linux
#endif
    system(command.c_str());
}

} // namespace lcs::net
