#pragma once

#include <cpprest/http_client.h>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>

class HttpRequestProcessor {

public:
    struct RequestItem {
        web::http::http_request Request;
        std::string Hint; // For logging/tracking
    };

    HttpRequestProcessor(std::wstring apiBaseUrl,
                         std::wstring universalToken);

    DISALLOW_COPY_MOVE(HttpRequestProcessor);

    ~HttpRequestProcessor();

    bool EnqueueRequest(
        const web::http::http_request & request,
        const std::string & deviceId);

private:
    void ProcessingWorker();
    static bool SendRequest(const RequestItem & item, const std::wstring & apiUrl);
    RequestItem CreateAwakingRequest() const;


private:
    std::wstring apiBaseUrl_;
	std::wstring universalToken_;
    std::queue<RequestItem> requestQueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread workerThread_;
    std::atomic<bool> running_;
	uint64_t retryAwakingCount_ = 0;
    static constexpr uint64_t MAX_AWAKING_RETRIES = 20;
    static constexpr uint64_t MAX_IGNORING_RETRIES = MAX_AWAKING_RETRIES * 3;
};
