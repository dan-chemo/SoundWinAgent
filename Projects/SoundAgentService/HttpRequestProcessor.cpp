#include "stdafx.h"
// ReSharper disable CppExpressionWithoutSideEffects

#include "HttpRequestProcessor.h"

#include <TimeUtils.h>
#include <nlohmann/json.hpp>

#include "FormattedOutput.h"


HttpRequestProcessor::HttpRequestProcessor(std::wstring apiBaseUrl,
    std::wstring universalToken)
    : apiBaseUrl_(std::move(apiBaseUrl))
    , universalToken_(std::move(universalToken))
    , running_(true)
{
    workerThread_ = std::thread(&HttpRequestProcessor::ProcessingWorker, this);
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

    // Add to queue
    requestQueue_.push(RequestItem{.Request = request, .DeviceId = deviceId});

    // Notify worker thread
    condition_.notify_one();
	return true;
}

bool HttpRequestProcessor::SendRequest(const RequestItem & item, const std::wstring & apiUrl)
{
    const auto messageDeviceAppendix = item.DeviceId.empty() ? "" : " for device: " + item.DeviceId;

    try
    {
        SPD_L->info("Processing request{}", messageDeviceAppendix);

        // Send request and handle response
        web::http::client::http_client client(apiUrl);

        const pplx::task<web::http::http_response> responseTask = client.request(item.Request);
        responseTask.then([messageDeviceAppendix](const web::http::http_response & response)
        {
            if (response.status_code() == web::http::status_codes::Created ||
                response.status_code() == web::http::status_codes::OK ||
                response.status_code() == web::http::status_codes::NoContent)
            {
                const auto msg = "Posted successfully" + messageDeviceAppendix;
                FormattedOutput::LogAndPrint(msg);
            }
            else
            {
                const auto statusCode = response.status_code();
				const auto msg = "Failed to post data" + messageDeviceAppendix +
                    " - Status code: " + std::to_string(statusCode);
                FormattedOutput::LogAndPrint(msg);
            }
        }).wait();
    }
    catch (const web::http::http_exception & ex)
    {
        const auto msg = "HTTP exception" + messageDeviceAppendix + ": " + std::string(ex.what());
        FormattedOutput::LogAndPrint(msg);
        return false;
    }
    catch (const std::exception & ex)
    {
        const auto msg = "Common exception while sending HTTP request" + messageDeviceAppendix + ": " +
            std::string(ex.what());
        FormattedOutput::LogAndPrint(msg);
    }
    catch (...)
    {
        const auto msg = "Unspecified exception while sending HTTP request" + messageDeviceAppendix;
        FormattedOutput::LogAndPrint(msg);
    }
    return true;
}

void HttpRequestProcessor::ProcessingWorker()
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
        }

        if (SendRequest(item, apiBaseUrl_))
        {
            std::unique_lock lock(mutex_);
            requestQueue_.pop();
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        SendRequest(
			CreateAwakingRequest()
            , L"https://api.github.com/user/codespaces/studious-bassoon-7vp9wvpw7rxjf4wg/start");

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

}

HttpRequestProcessor::RequestItem HttpRequestProcessor::CreateAwakingRequest() const
{
    const nlohmann::json payload = {
        // ReSharper disable once StringLiteralTypo
        {"codespace_name", "studious-bassoon-7vp9wvpw7rxjf4wg"}
    };
    // Convert nlohmann::json to cpprestsdk::json::value
    const web::json::value jsonPayload = web::json::value::parse(payload.dump());

    const std::wstring authorizationValue = L"Bearer " + universalToken_;

    web::http::http_request request(web::http::methods::POST);
    request.headers().add(U("Authorization"), authorizationValue);
    request.headers().add(U("Accept"), U("application/vnd.github.v3+json"));
    request.headers().set_content_type(U("application/json"));
    request.set_body(jsonPayload);

    return RequestItem{ request, "" };
}


