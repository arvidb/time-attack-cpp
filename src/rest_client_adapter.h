#pragma once

#include <cpp-httplib/httplib.h>

namespace timeattack
{
    enum class RequestMethod {
        GET, POST
    };
    
    struct RestClientAdapterResponse {
        int status;
        std::string body;
        
        explicit RestClientAdapterResponse(const httplib::Response& res) noexcept
            : status(res.status), body(res.body)
        {}
    };
    
    class RestClientAdapter {
        
    public:
        RestClientAdapter(const std::string& host, const int port, const int timeout)
            : _cli(host.c_str(), port, timeout)
        {}
        
        const std::unique_ptr<RestClientAdapterResponse> ExecuteRequest(const RequestMethod method, const std::string& endpoint, const std::string& postData) {
            
            std::shared_ptr<httplib::Response> response;
            
            if (method == RequestMethod::GET) {
                if ((response = _cli.Get(endpoint.c_str())) == nullptr) {
                    throw std::runtime_error("Error while performing GET request");
                }
            }
            else if (method == RequestMethod::POST) {
                if ((response = _cli.Post(endpoint.c_str(), postData, "application/x-www-form-urlencoded")) == nullptr) {
                    throw std::runtime_error("Error while performing POST request");
                }
            }
            
            return response != nullptr
                ? std::make_unique<RestClientAdapterResponse>(*response)
                : nullptr;
        };
        
    private:
        httplib::Client _cli;
    };
}