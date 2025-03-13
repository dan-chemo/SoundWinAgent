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
        std::string DeviceId; // For logging/tracking
    };

	explicit HttpRequestProcessor(std::wstring apiBaseUrl, int minIntervalSeconds = 5);

    DISALLOW_COPY_MOVE(HttpRequestProcessor);

    ~HttpRequestProcessor();

    // Add request to the queue
    void EnqueueRequest(
        const web::http::http_request& request,
        const std::string& deviceId);

private:
    void ProcessRequests();

private:
    std::wstring apiBaseUrl_;

    std::queue<RequestItem> requestQueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread workerThread_;
    std::atomic<bool> running_;
    std::chrono::seconds minInterval_;
    std::chrono::system_clock::time_point lastRequestTime_;
};
