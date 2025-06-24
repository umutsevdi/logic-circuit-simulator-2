#include "common.h"
#include "io.h"
#include "net.h"
#include "ui/configuration.h"
#include <base64.h>
#include <json/reader.h>
#include <keychain/keychain.h>
#include <ctime>
#include <filesystem>

namespace lcs::net {

struct AuthInfo {
    Account account;
    std::string access_token = "";
};
AuthInfo _auth;
static AuthenticationFlow _flow;

Error static _get_client_id(std::string& id)
{
    std::string resp;
    Error err
        = get_request(ui::get_config().api_proxy + "/api/client_id", resp);
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
    return OK;
}

Error AuthenticationFlow::start(void)
{
    if (_last_status == DONE) {
        return OK;
    } else if (_last_status != INACTIVE) {
        return ERROR(Error::UNTERMINATED_FLOW);
    }
    _auth        = {};
    _last_status = STARTED;
    L_INFO("Starting flow");
    std::string _id;
    Error err = _get_client_id(_id);
    if (err) {
        _last_status = BROKEN;
        return err;
    }
    std::string body;
    err = net::post_request("https://github.com/login/device/code?client_id="
            + _id + "&scope=read:user",
        body);

    if (err) {
        _last_status = BROKEN;
        return err;
    }
    Json::Value v;
    Json::Reader r;
    if (!r.parse(body, v)) {
        _last_status = BROKEN;
        return ERROR(Error::JSON_PARSE_ERROR);
    }
    L_INFO("Received: " << v.toStyledString());
    client_id        = _id;
    device_code      = v["device_code"].asString();
    user_code        = v["user_code"].asString();
    verification_uri = v["verification_uri"].asString();
    expires_in       = v["expires_in"].asInt();
    interval         = v["interval"].asInt();
    start_time       = time(nullptr);
    _last_poll       = start_time;

    L_INFO("Visit: https://github.com/login/device");
    L_INFO("Enter the code: " << v["user_code"].asString());
    open_browser(verification_uri);
    return OK;
}

Flow::State AuthenticationFlow::poll(void)
{
    if (_last_status == STARTED) {
        _last_status = POLLING;
    }
    if (_last_status == BROKEN || _last_status == TIMEOUT) {
        return _last_status;
    }
    time_t now = time(nullptr);
    if (difftime(now, this->start_time) > this->expires_in) {
        _last_status = (ERROR(Flow::State::TIMEOUT));
        return _last_status;
    }
    if (difftime(now, this->_last_poll) < this->interval * 1.5) {
        _last_status = Flow::State::POLLING;
        return Flow::State::POLLING;
    }
    this->_last_poll = now;

    {
        L_INFO("Get oauth access token");
        Json::Value req;
        req["client_id"]   = client_id;
        req["device_code"] = device_code;
        req["grant_type"]  = "urn:ietf:params:oauth:grant-type:device_code";
        L_INFO("Sending:" << req.toStyledString());

        std::string gh_resp_str;
        Error err = post_request("https://github.com/login/oauth/access_token",
            gh_resp_str, req.toStyledString());
        if (err) {
            _reason = gh_resp_str;
            L_WARN("Received " << _reason);
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }

        Json::Value gh_resp;
        Json::Reader gh_resp_r {};
        if (!gh_resp_r.parse(gh_resp_str, gh_resp)) {
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }
        if (gh_resp["error"].isString()
            && gh_resp["error"].asString() == "authorization_pending") {
            // TODO For other types of error codes change the to
            // Flow::State::BROKEN
            _last_status = Flow::State::POLLING;
            return _last_status;
        } else if (gh_resp["error"].isString()) {
            _reason = gh_resp["error_description"].asString();
        }
        L_INFO("Received " << gh_resp.toStyledString());
        _auth.access_token = gh_resp["access_token"].asString();
    }

    {
        L_INFO("Send token to session server");
        std::string session_resp;
        Error err = get_request(ui::get_config().api_proxy + "/api/login",
            session_resp, _auth.access_token);
        if (err) {
            _reason = session_resp;
            L_WARN("Received " << _reason);
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }
        Json::Value resp_root;
        Json::Reader resp {};
        if (!resp.parse(session_resp, resp_root)) {
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }
        L_INFO("Received: " << resp_root.toStyledString());
        if (!resp_root["login"].isString()) {
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }

        _auth.account.login      = resp_root["login"].asString();
        _auth.account.name       = resp_root["name"].asString();
        _auth.account.email      = resp_root["email"].asString();
        _auth.account.bio        = resp_root["bio"].asString();
        _auth.account.url        = resp_root["url"].asString();
        _auth.account.avatar_url = resp_root["avatarUrl"].asString();
        keychain::Error keyerr;
        keychain::setPassword(APP_PKG, APPNAME_LONG, _auth.account.login,
            _auth.access_token, keyerr);
        if (keyerr.type != keychain::ErrorType::NoError) {
            L_WARN(keyerr.message);
        }
        io::write(io::ROOT / ".login", _auth.account.login);

        if (!std::filesystem::exists(io::CACHE
                / (base64_encode(_auth.account.avatar_url) + ".jpeg"))) {
            std::vector<unsigned char> data;
            net::get_request(_auth.account.avatar_url, data);
            if (!data.empty()) {
                io::write(io::CACHE
                        / (base64_encode(_auth.account.avatar_url) + ".jpeg"),
                    data);
            }
        }
    }

    _last_status = Flow::State::DONE;
    return Flow::State::DONE;
}

Flow::State AuthenticationFlow::get_state(void) const { return _last_status; }

const char* AuthenticationFlow::reason(void) const { return _reason.c_str(); }

Error AuthenticationFlow::start_existing(void)
{
    _last_status      = STARTED;
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
    Error err
        = get_request(ui::get_config().api_proxy + "/api/login", resp_str, pwd);
    if (err) {
        _reason = resp_str;
        L_WARN("Received " << _reason);
        return err;
    }
    Json::Value resp_root;
    Json::Reader resp {};
    if (!resp.parse(resp_str, resp_root)) {
        return ERROR(Error::JSON_PARSE_ERROR);
    }
    L_INFO("Received: " << resp_root.toStyledString());
    _auth.access_token       = pwd;
    _auth.account.login      = resp_root["login"].asString();
    _auth.account.name       = resp_root["name"].asString();
    _auth.account.email      = resp_root["email"].asString();
    _auth.account.bio        = resp_root["bio"].asString();
    _auth.account.url        = resp_root["url"].asString();
    _auth.account.avatar_url = resp_root["avatarUrl"].asString();

    if (!std::filesystem::exists(
            io::CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"))) {
        std::vector<unsigned char> data;
        net::get_request(_auth.account.avatar_url, data);
        if (!data.empty()) {
            io::write(
                io::CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"),
                data);
        }
    }
    _last_status = DONE;
    return OK;
}

void AuthenticationFlow::resolve(void)
{
    _reason      = "";
    _auth        = {};
    _last_status = INACTIVE;
}

AuthenticationFlow& get_flow(void) { return _flow; }

const Account& get_account(void) { return _auth.account; }

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

Error upload_scene(NRef<const Scene> scene, std::string resp)
{
    return net::post_request(ui::get_config().api_proxy + "/api/upload", resp,
        to_json<Scene>(*scene).toStyledString(), _auth.access_token);
}

} // namespace lcs::net
