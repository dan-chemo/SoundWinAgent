#include "stdafx.h"
// ReSharper disable CppExpressionWithoutSideEffects

#include "HttpRequestProcessor.h"

HttpRequestProcessor::HttpRequestProcessor(std::wstring apiBaseUrl, int minIntervalSeconds/* = 5*/)
    : apiBaseUrl_(std::move(apiBaseUrl))
      , running_(true)
      , minInterval_(std::chrono::seconds(minIntervalSeconds))
      , lastRequestTime_(std::chrono::system_clock::now() - minInterval_)
{
    workerThread_ = std::thread(&HttpRequestProcessor::ProcessRequests, this);
}

HttpRequestProcessor::~HttpRequestProcessor()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        condition_.notify_all();
    }
    if (workerThread_.joinable())
    {
        workerThread_.join();
    }
}

void HttpRequestProcessor::EnqueueRequest(const web::http::http_request & request, const std::string & deviceId)
{
    std::unique_lock lock(mutex_);

    // Create RequestItem
    RequestItem item{request, deviceId};

    // Add to queue
    requestQueue_.push(std::move(item));

    // Notify worker thread
    condition_.notify_one();
}

void HttpRequestProcessor::ProcessRequests()
{
    while (true)
    {
        RequestItem item;

        // Wait for a request or shutdown signal
        {
            std::unique_lock lock(mutex_);

            condition_.wait(lock, [this]
            {
                return !running_ || !requestQueue_.empty();
            });

            // Check if we're shutting down
            if (!running_ && requestQueue_.empty())
            {
                break;
            }

            // Check if enough time has passed since last request
            auto now = std::chrono::system_clock::now();

            if (auto elapsed = now - lastRequestTime_
                ; elapsed < minInterval_)
            {
                // Not enough time has passed, skip this request
                if (requestQueue_.empty())
                {
                    continue;
                }

                SPD_L->info("Skipping request for device: {} (too soon after previous request)",
                            requestQueue_.front().DeviceId);
                requestQueue_.pop();
                continue;
            }

            // Get the next request from the queue
            if (requestQueue_.empty())
            {
                continue;
            }

            item = std::move(requestQueue_.front());
            requestQueue_.pop();

            // Update last request time
            lastRequestTime_ = now;
        }

        // Process request outside of lock
        try
        {
            SPD_L->info("Processing request for device: {}", item.DeviceId);

            // Send request and handle response
            web::http::client::http_client client(apiBaseUrl_);

            const pplx::task<web::http::http_response> responseTask = client.request(item.Request);
            responseTask.then([deviceId = item.DeviceId](const web::http::http_response & response)
            {
                if (response.status_code() == web::http::status_codes::Created ||
                    response.status_code() == web::http::status_codes::OK ||
                    response.status_code() == web::http::status_codes::NoContent)
                {
                    const auto msg = "Device data posted successfully for: " + deviceId;
                    FormattedOutput::LogAndPrint(msg);
                }
                else
                {
                    const auto statusCode = response.status_code();
                    const auto msg = "Failed to post device data for: " + deviceId +
                        " - Status code: " + std::to_string(statusCode);
                    FormattedOutput::LogAndPrint(msg);
                }
            }).wait();
        }
        catch (const web::http::http_exception & ex)
        {
            const auto msg = "HTTP exception for device " + item.DeviceId + ": " + std::string(ex.what());
            FormattedOutput::LogAndPrint(msg);
        }
        catch (const std::exception & ex)
        {
            const auto msg = "Common exception while sending HTTP request for device " + item.DeviceId + ": " +
                std::string(ex.what());
            FormattedOutput::LogAndPrint(msg);
        }
        catch (...)
        {
            const auto msg = "Unspecified exception while sending HTTP request for device " + item.DeviceId;
            FormattedOutput::LogAndPrint(msg);
        }
    }
}
