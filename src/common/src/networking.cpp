#include <condition_variable>
#include <curl/curl.h>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "common.h"
#include "port.h"

namespace lcs::net {

static size_t _write_cb(
    void* contents, size_t size, size_t nmemb, std::vector<unsigned char>& resp)
{
    size_t total = size * nmemb;
    resp.insert(resp.end(), static_cast<unsigned char*>(contents),
        static_cast<unsigned char*>(contents) + total);
    return total;
}

enum HttpSessionFlags : uint8_t {
    IS_PROCESSED = 1 << 0,
    IS_RESOLVED  = 1 << 1,
    IS_POST      = 1 << 2,
    IS_TERM      = 1 << 3,
};

class HttpHandle {
public:
    HttpHandle() = default;
    // Get request constructor
    void get(const std::string& url, const std::string& auth = "",
        std::function<void(HttpResponse& resp)> _callback = nullptr);
    // Post request constructor
    void post(const std::string& url, const std::vector<uint8_t>& _req,
        const std::string& auth                           = "",
        std::function<void(HttpResponse& resp)> _callback = nullptr);
    void term(void);

    void handle(void);
    bool is_done() const;

    HttpResponse resp;
    std::function<void(HttpResponse& resp)> cb = nullptr;
    uint8_t flags                              = 0U;
    const char* url() const { return _url.c_str(); }

private:
    // Request Data
    std::string _url;
    std::string _auth;
    std::vector<uint8_t> _req;
    // Response Data
};

void HttpHandle::get(const std::string& url, const std::string& auth,
    std::function<void(HttpResponse& resp)> callback)
{
    _url  = url;
    _auth = auth;
    cb    = callback;
    L_DEBUG("Sending GET request.");
}

// Post request constructor
void HttpHandle::post(const std::string& url, const std::vector<uint8_t>& req,
    const std::string& auth, std::function<void(HttpResponse& resp)> callback)
{
    _url  = url;
    _auth = auth;
    _req  = req;
    flags |= IS_POST;
    cb = callback;
    L_DEBUG("Sending POST request.");
}

void HttpHandle::term()
{
    flags |= IS_TERM;
    L_DEBUG("Sending TERM request.");
}

void HttpHandle::handle(void)
{
    if (flags & (IS_RESOLVED | IS_PROCESSED)) {
        return;
    }
    flags |= IS_PROCESSED;

    L_DEBUG(
        "%s %s\t[Auth: %s]", flags & IS_POST ? "POST" : "GET", _url.c_str());
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp.data.clear();
        resp.err = ERROR(Error::REQUEST_FAILED);
        flags |= IS_RESOLVED;
        return;
    }
    curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (!_req.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _req.data());
    }
    if (~flags & IS_POST) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    }
    struct curl_slist* headers = nullptr;
    if (!_auth.empty()) {
        std::string authHeader = "Authorization: Bearer " + _auth;
        headers                = curl_slist_append(headers, authHeader.c_str());
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res_code = curl_easy_perform(curl);
    if (res_code != CURLE_OK) {
        curl_easy_cleanup(curl);
        resp.err = ERROR(Error::REQUEST_FAILED);
        flags |= IS_RESOLVED;
        return;
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
    L_DEBUG("Received: %d Payload: %s", resp.status_code, resp.data.data());

    if (resp.status_code != 200) {
        curl_easy_cleanup(curl);
        resp.err = ERROR(Error::RESPONSE_ERROR);
        flags |= IS_RESOLVED;
        return;
    }
    flags |= IS_RESOLVED;
    resp.err = Error::OK;
    if (cb != nullptr) {
        cb(resp);
    }
}

bool HttpHandle::is_done() const
{
    return flags & (IS_RESOLVED | IS_PROCESSED);
}

std::map<uint64_t, HttpHandle> data {};
uint64_t last = 0;
std::mutex data_mutex;
std::condition_variable cv;
static std::thread _thread;

void init(bool)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        throw std::runtime_error("Failed to initialize cURL: "
            + std::string(curl_easy_strerror(res)));
    }
    _thread = std::thread { [&]() {
        L_INFO("Module lcs::net is ready");
        bool term = false;
        while (true) {
            std::vector<size_t> erase_list;
            {
                std::unique_lock<std::mutex> lock(data_mutex);
                cv.wait(lock, [&] { return !data.empty(); });
                for (auto& [k, v] : data) {
                    if (v.flags & IS_TERM) {
                        term = true;
                        continue;
                    }
                    v.handle();
                    if (v.cb != nullptr) {
                        erase_list.push_back(k);
                    }
                }
                for (auto& it : erase_list) {
                    data.erase(it);
                }
            }
            if (term) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } };
}

void close(void)
{
    {
        std::unique_lock<std::mutex> lock(data_mutex);
        data.emplace(UINT64_MAX, HttpHandle {});
        data[UINT64_MAX].term();
        cv.notify_one();
    }
    if (_thread.joinable()) {
        _thread.join();
    }
    L_INFO("Module lcs::net is closed.");
}

uint64_t get_request(const std::string& URL, const std::string& authorization)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    last++;
    data.emplace(last, HttpHandle {});
    data[last].get(URL, authorization);
    cv.notify_one();
    return last;
}

void get_request_then(std::function<void(HttpResponse& resp)> cb,
    const std::string& URL, const std::string& authorization)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    last++;
    data.emplace(last, HttpHandle {});
    data[last].get(URL, authorization, cb);
    cv.notify_one();
}

uint64_t post_request(const std::string& URL, const std::vector<uint8_t>& req,
    const std::string& authorization)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    last++;
    data.emplace(last, HttpHandle {});
    data[last].post(URL, req, authorization);
    cv.notify_one();
    return last;
}

void post_request_then(std::function<void(HttpResponse& resp)> cb,
    const std::string& URL, const std::vector<uint8_t>& req,
    const std::string& authorization)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    last++;
    data.emplace(last, HttpHandle {});
    data[last].post(URL, req, authorization, cb);
    cv.notify_one();
}

bool pull_response(uint64_t id, HttpResponse& resp)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    if (auto it = data.find(id); it != data.end()) {
        if (it->second.is_done()) {
            auto n = data.extract(it);
            resp   = std::move(n.mapped().resp);
            return true;
        }
    }
    return false;
}

} // namespace lcs::net
