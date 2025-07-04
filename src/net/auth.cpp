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

namespace gh {
    LCS_ERROR
    static _get_device_code(Json::Value& response, const std::string& _id)
    {
        Error err = OK;
        ;
        std::string body;
        err = net::post_request(
            "https://github.com/login/device/code?client_id=" + _id
                + "&scope=read:user",
            body);
        L_INFO("Received: %s", body.c_str());
        if (err) {
            return err;
        }
        Json::Reader parser;
        if (!parser.parse(body, response)) {
            return ERROR(Error::JSON_PARSE_ERROR);
        }
        return err;
    }

    LCS_ERROR
    static _get_access_token(Json::Value& response,
        const std::string& client_id, const std::string& device_code)
    {
        L_INFO("Get oauth access token");
        Json::Value req;
        req["client_id"]   = client_id;
        req["device_code"] = device_code;
        req["grant_type"]  = "urn:ietf:params:oauth:grant-type:device_code";
        L_INFO("Sending: %s", req.toStyledString().c_str());

        std::string body;
        Error err = post_request("https://github.com/login/oauth/access_token",
            body, req.toStyledString());
        L_INFO("Received %s", body.c_str());
        Json::Reader parser {};
        if (!parser.parse(body, response)) {
            return ERROR(Error::JSON_PARSE_ERROR);
        }
        if (err) {
            return err;
        }
        return OK;
    }

}; // namespace gh
namespace api {
    LCS_ERROR
    static _get_client_id(Json::Value& response)
    {
        std::string resp;
        Error err
            = get_request(ui::get_config().api_proxy + "/api/client_id", resp);
        L_INFO("Received %s", resp.c_str());
        if (err) {
            return err;
        }
        Json::Reader parser;
        if (!parser.parse(resp, response)) {
            return ERROR(Error::JSON_PARSE_ERROR);
        }
        return OK;
    }

    LCS_ERROR
    static _login(Json::Value& response, const std::string& token = "")
    {
        L_INFO("Send token to session server");
        std::string resp;
        Error err = get_request(
            ui::get_config().api_proxy + "/api/login", resp, token);
        L_INFO("Received: %s", resp.c_str());
        if (err) {
            return err;
        }
        Json::Reader parser {};
        if (!parser.parse(resp, response)) {
            return ERROR(Error::JSON_PARSE_ERROR);
        }
        return OK;
    }
} // namespace api

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
    Json::Value v;
    Error err = api::_get_client_id(v);
    if (err) {
        _last_status = BROKEN;
        return err;
    }
    std::string _id = v["id"].asString();
    err             = gh::_get_device_code(v, _id);
    if (err) {
        _last_status = BROKEN;
        return err;
    }
    client_id        = _id;
    device_code      = v["device_code"].asString();
    user_code        = v["user_code"].asString();
    verification_uri = v["verification_uri"].asString();
    expires_in       = v["expires_in"].asInt();
    interval         = v["interval"].asInt();
    start_time       = time(nullptr);
    _last_poll       = start_time;

    L_INFO("Visit: https://github.com/login/device");
    L_INFO("Enter the code: %s", v["user_code"].asCString());
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
        Json::Value resp;
        Error err = gh::_get_access_token(resp, client_id, device_code);
        if (err) {
            if (resp["error"].isString()) {
                _reason = resp["error"].asString();
            }
            L_WARN("Received %s", _reason);
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }

        if (resp["error"].isString()
            && resp["error"].asString() == "authorization_pending") {
            // TODO For other types of error codes change the to
            // Flow::State::BROKEN
            _last_status = Flow::State::POLLING;
            return _last_status;
        } else if (resp["error"].isString()) {
            _reason = resp["error_description"].asString();
        }
        _auth.access_token = resp["access_token"].asString();
    }

    {
        Json::Value v;
        Error err = api::_login(v, _auth.access_token);
        if (err) {
            if (v["message"].isString()) {
                _reason = v["message"].asString();
            }
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }
        if (!v["login"].isString()) {
            _last_status = (ERROR(Flow::State::BROKEN));
            return _last_status;
        }

        _auth.account.login      = v["login"].asString();
        _auth.account.name       = v["name"].asString();
        _auth.account.email      = v["email"].asString();
        _auth.account.bio        = v["bio"].asString();
        _auth.account.url        = v["url"].asString();
        _auth.account.avatar_url = v["avatarUrl"].asString();
        keychain::Error keyerr;
        keychain::setPassword(APP_PKG, APPNAME_LONG, _auth.account.login,
            _auth.access_token, keyerr);
        if (keyerr.type != keychain::ErrorType::NoError) {
            L_WARN("%s", keyerr.message);
        }
        write(ROOT / ".login", _auth.account.login);

        if (!std::filesystem::exists(
                CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"))) {
            std::vector<unsigned char> data;
            if (net::get_request(_auth.account.avatar_url, data) == OK
                && !data.empty()) {
                write(
                    CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"),
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
    std::string login = read(ROOT / ".login");
    if (login == "") {
        return ERROR(Error::KEYCHAIN_NOT_FOUND);
    }
    keychain::Error keyerr;
    std::string pwd
        = keychain::getPassword(APP_PKG, APPNAME_LONG, login, keyerr);
    if (keyerr.type != keychain::ErrorType::NoError) {
        L_WARN("%s", keyerr.message);
        return (Error)(keyerr + KEYCHAIN_GENERIC_ERROR - 1);
    }

    Json::Value v;
    Error err = api::_login(v, pwd);
    if (err) {
        if (v["message"].isString()) {
            _reason = v["message"].asString();
        }
        return err;
    }
    _auth.access_token       = pwd;
    _auth.account.login      = v["login"].asString();
    _auth.account.name       = v["name"].asString();
    _auth.account.email      = v["email"].asString();
    _auth.account.bio        = v["bio"].asString();
    _auth.account.url        = v["url"].asString();
    _auth.account.avatar_url = v["avatarUrl"].asString();

    if (!std::filesystem::exists(
            CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"))) {
        std::vector<unsigned char> data;
        if (net::get_request(_auth.account.avatar_url, data) == OK
            && !data.empty()) {
            write(CACHE / (base64_encode(_auth.account.avatar_url) + ".jpeg"),
                data);
        }
    }
    _last_status = DONE;
    return OK;
}

void AuthenticationFlow::resolve(void)
{
    if (_auth.account.login != "") {
        keychain::Error err;
        keychain::deletePassword(
            APP_PKG, APPNAME_LONG, _auth.account.login, err);
        if (err.type != keychain::ErrorType::NoError) {
            L_ERROR("Error while cleaning password. %s", err.message.c_str());
        }
        write(ROOT / ".login", "");
    }
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
    return net::post_request(ui::get_config().api_proxy + "/api/scene", resp,
        to_json<Scene>(*scene).toStyledString(), _auth.access_token);
}

} // namespace lcs::net
