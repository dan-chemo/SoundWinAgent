#include "stdafx.h"
// ReSharper disable CppExpressionWithoutSideEffects

#include "HttpRequestProcessor.h"

#include <TimeUtils.h>

//#include <spdlog/fmt/ostr.h>
#include "FormattedOutput.h"


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
        std::unique_lock lock(mutex_);
        running_ = false;
        condition_.notify_all();
    }
    if (workerThread_.joinable())
    {
        workerThread_.join();
    }
}

bool HttpRequestProcessor::EnqueueRequest(const web::http::http_request & request, const std::string & deviceId)
{
    std::unique_lock lock(mutex_);

    // Check if enough time has passed since last request
    const auto now = std::chrono::system_clock::now();
    const auto elapsed = now - lastRequestTime_;
    if (elapsed < minInterval_)
    {
        SPD_L->info("Current time is {}.", ed::systemTimeToStringWithLocalTime(now, std::string(" ")));
        SPD_L->info("Previous request's time was {}.", ed::systemTimeToStringWithLocalTime(lastRequestTime_, std::string(" ")));
        SPD_L->info("Skipping request for device: {0}, because it comes less then in {1} sec after previous request.", deviceId, minInterval_.count());
        return false;
    }

    // Update last request time
    lastRequestTime_ = now;

    // Add to queue
    requestQueue_.push(RequestItem{request, deviceId});

    // Notify worker thread
    condition_.notify_one();
	return true;
}

void HttpRequestProcessor::SendRequest(RequestItem item, const std::wstring& apiUrl)
{
    try
    {
        SPD_L->info("Processing request for device: {}", item.DeviceId);

        // Send request and handle response
        web::http::client::http_client client(apiUrl);

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

void HttpRequestProcessor::ProcessRequests()
{
    while (true)
    {
        RequestItem item;

        {
            std::unique_lock lock(mutex_);

            condition_.wait(lock, [this]
            {
                return !running_ || !requestQueue_.empty();
            });

            // Check if we're shutting down
            if (!running_) // && requestQueue_.empty())
            {
                break;
            }

            if (requestQueue_.empty())
            {
                continue;
            }

            item = std::move(requestQueue_.front());
            requestQueue_.pop();
        }

        SendRequest(item, apiBaseUrl_);
    }
}
