#include "common.h"
#include "net.h"
#include "json/reader.h"
#include <ctime>

namespace lcs::net {

Account account;
std::string access_token = "";
bool _is_logged_in       = false;

std::string get_client_id(void)
{
    std::string resp;
    get(Packet { "http://localhost:8080/api/client_id", "" }, resp);
    return resp;
}

Error start_device_flow(DeviceFlow& flow)
{
    L_INFO("Calling flow");
    std::string client_id = get_client_id();
    if (client_id == "") {
        return ERROR(Error::CURL_RESPONSE_ERROR);
    }
    std::string body;
    Error err
        = net::post(Packet { "https://github.com/login/device/code?client_id="
                            + client_id + "&scope=read:user",
                        "" },
            "", body);

    if (err) {
        return ERROR(err);
    }
    L_INFO("Received: " << body);
    Json::Value v;
    Json::Reader r;
    if (!r.parse(body, v)) {
        return S_ERROR("Parse error", Error::CURL_REQUEST_FAILED);
    }
    L_INFO("Received: " << v.toStyledString());
    flow.client_id        = client_id;
    flow.device_code      = v["device_code"].asString();
    flow.user_code        = v["user_code"].asString();
    flow.verification_uri = v["verification_uri"].asString();
    flow.expires_in       = v["expires_in"].asInt();
    flow.interval         = v["interval"].asInt();
    flow.start_time       = time(0);
    flow._last_poll       = flow.start_time;

    L_INFO("Visit: https://github.com/login/device");
    L_INFO("Enter the code: " << v["user_code"].asString());
    open_browser(flow.verification_uri);
    return SUCCESS;
}

DeviceFlow::PollState DeviceFlow::poll(void)
{
    time_t now = time(nullptr);
    if (difftime(now, this->start_time) > this->expires_in) {
        return ERROR(DeviceFlow::PollState::TIMEOUT);
    }
    if (difftime(now, this->_last_poll) < this->interval * 1.5) {
        return DeviceFlow::PollState::POLLING;
    }
    this->_last_poll = now;
    Json::Value v;
    v["client_id"]   = client_id;
    v["device_code"] = device_code;
    v["grant_type"]  = "urn:ietf:params:oauth:grant-type:device_code";

    std::string resp;
    L_INFO("Sending:" << v.toStyledString());
    Error err
        = post(Packet { "https://github.com/login/oauth/access_token", "" },
            v.toStyledString(), resp);
    if (err) {
        return ERROR(DeviceFlow::PollState::BROKEN);
    }
    Json::Value resp_root;
    Json::Reader r {};
    if (!r.parse(resp, resp_root)) {
        return ERROR(DeviceFlow::PollState::BROKEN);
    }
    if (resp_root["error"].isString()
        && resp_root["error"].asString() == "authorization_pending") {
        return DeviceFlow::PollState::POLLING;
    }
    L_INFO("Received " << resp_root.toStyledString());
    _is_logged_in = true;
    access_token  = resp_root["access_token"].asString();
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
