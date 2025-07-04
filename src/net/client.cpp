#include "common.h"
#include "net.h"
#include <curl/curl.h>
#include <stdexcept>
#include <string>

namespace lcs::net {

void init(void)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        throw std::runtime_error("Failed to initialize cURL: "
            + std::string(curl_easy_strerror(res)));
    }
}

static size_t _write_cb(
    void* data, size_t size, size_t nmemb, std::string* userp)
{
    size_t total = size * nmemb;
    userp->append(static_cast<char*>(data), total);
    return total;
}

Error get_request(
    const std::string& url, std::string& resp, const std::string& authorization)
{
    L_INFO("Sending GET to %s\tAuth: %s", url.c_str(), authorization.c_str());
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp = "";
        return ERROR(Error::REQUEST_FAILED);
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(
        curl, CURLOPT_HTTPHEADER, nullptr); // Set headers if needed
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    struct curl_slist* headers = nullptr;
    if (!authorization.empty()) {
        std::string authHeader = "Authorization: Bearer " + authorization;
        headers                = curl_slist_append(headers, authHeader.c_str());
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(Error::REQUEST_FAILED);
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        resp = readBuffer;
        L_INFO("Received: %d Payload: %s", response_code, readBuffer.c_str());
        curl_easy_cleanup(curl);
        return ERROR(Error::RESPONSE_ERROR);
    }

    resp = readBuffer;
    curl_easy_cleanup(curl);
    return Error::OK;
}

Error post_request(const std::string& url, std::string& resp,
    const std::string& req, const std::string& authorization)
{
    L_INFO("Sending POST to %s\tAuth:%s\tReq: %s", url.c_str(),
        authorization.c_str(), req.c_str());
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp = "";
        return ERROR(Error::REQUEST_FAILED);
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    struct curl_slist* headers = nullptr;
    if (!authorization.empty()) {
        std::string authHeader = "Authorization: Bearer " + authorization;
        headers                = curl_slist_append(headers, authHeader.c_str());
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    if (req != "") {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(Error::REQUEST_FAILED);
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        L_INFO("Received: %d Payload: %s", response_code, readBuffer.c_str());
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(Error::RESPONSE_ERROR);
    }

    resp = readBuffer;
    curl_easy_cleanup(curl);
    return Error::OK;
}

static size_t write_bin_cb(
    void* contents, size_t size, size_t nmemb, std::vector<unsigned char>& resp)
{
    size_t total = size * nmemb;
    resp.insert(resp.end(), static_cast<unsigned char*>(contents),
        static_cast<unsigned char*>(contents) + total);
    return total;
}

Error get_request(const std::string& url, std::vector<unsigned char>& resp,
    const std::string&)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_bin_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            return ERROR(Error::REQUEST_FAILED);
        }
        curl_easy_cleanup(curl);
    }
    return Error::OK;
}

} // namespace lcs::net
