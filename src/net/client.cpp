#include "common.h"
#include <curl/curl.h>
#include <stdexcept>
#include <string>

#include "net.h"

namespace lcs::net {

void init(void)
{
    // Initialize cURL
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        throw std::runtime_error("Failed to initialize cURL: "
            + std::string(curl_easy_strerror(res)));
    }
}

size_t WriteCallback(
    void* contents, size_t size, size_t nmemb, std::string* userp)
{
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

Error get(const Packet& pkt, std::string& resp)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp = "";
        return ERROR(CURL_INIT_FAILED);
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, pkt.url.c_str());
    curl_easy_setopt(
        curl, CURLOPT_HTTPHEADER, nullptr); // Set headers if needed
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    struct curl_slist* headers = nullptr;
    if (!pkt.session_token.empty()) {
        std::string authHeader = "Authorization: Bearer " + pkt.session_token;
        headers                = curl_slist_append(headers, authHeader.c_str());
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(CURL_REQUEST_FAILED);
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(CURL_RESPONSE_ERROR);
    }

    resp = readBuffer;
    curl_easy_cleanup(curl);
    return ERROR(SUCCESS);
}

Error post(const Packet& pkt, const std::string& req, std::string& resp)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp = "";
        return ERROR(CURL_INIT_FAILED);
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, pkt.url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    struct curl_slist* headers = nullptr;
    if (!pkt.session_token.empty()) {
        std::string authHeader = "Authorization: Bearer " + pkt.session_token;
        headers                = curl_slist_append(headers, authHeader.c_str());
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    if (req != "") {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        resp = "";
        return ERROR(CURL_REQUEST_FAILED);
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        L_INFO(readBuffer);
        resp = "";
        curl_easy_cleanup(curl);
        return ERROR(CURL_RESPONSE_ERROR);
    }

    resp = readBuffer;
    curl_easy_cleanup(curl);
    return SUCCESS;
}

} // namespace lcs::net
