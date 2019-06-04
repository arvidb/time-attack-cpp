#pragma once

#include "common.h"
#include "result_functions.h"

#include <cpp-httplib/httplib.h>

#include <chrono>
#include <numeric>
#include <algorithm>

#include <thread>
#include <future>

namespace timeattack {
    
    using ResultFunction = std::function<const double(std::vector<double>)>;
    
    constexpr int kDefaultSampleCount = 1;
    constexpr int kDefaultMaxConcurrentRequests = 1;
    constexpr int kDefaultPort = 80;
    constexpr int kDefaultTimeout = 5;
    
    enum class RequestMethod {
        GET, POST
    };
    
    struct Worker {
        
        struct WorkerTaskResult {
            std::string input;
            std::vector<double> samples;
        };
        
        
        Worker(const std::string& host, const int port = kDefaultPort, const int maxConcurrentRequests = kDefaultMaxConcurrentRequests)
        : _cli(host.c_str(), port, kDefaultTimeout),
        _sem(maxConcurrentRequests)
        {
            spdlog::debug("Creating worker for {}:{}", host, port);
        }
        
        /**
         * Creates all worker tasks for the params supplied
         */
        void DoWork(const std::vector<std::string>& params) {
            
            if (_bodyFmtTemplate.empty()) {
                
                spdlog::error("Body format template cannot be empty");
                return;
            }
            
            if (_apiPath.empty()) {
                
                spdlog::error("API path cannot be empty, specify a path starting with a leading slash \"/\"");
                return;
            }
            
            spdlog::info("Starting worker");
            spdlog::info("Body template: {}", _bodyFmtTemplate);
            spdlog::info("API path: {}", _apiPath);
            spdlog::info("Request method: {}",
                          _requestMethod == RequestMethod::GET ? "GET" : "POST");
            
            _results.clear();
            
            for (const auto& param : params) {
                
                spdlog::debug("Creating task for input: {}", param);
                
                auto task = std::async([this, param]() -> WorkerTaskResult {
                    
                    _sem.wait();
                    
                    spdlog::debug("Processing input: {} [samples: {}]", param, _sampleCount);
                    
                    std::vector<double> samples;
                    
                    try {
                        
                        for (int i=0; i < _sampleCount; i++) {
                            
                            const auto ExecuteRequest = [this, param](const RequestMethod method, const std::string& path) {
                                
                                std::shared_ptr<httplib::Response> response;
                                
                                if (method == RequestMethod::GET) {
                                    if ((response = _cli.Get(path.c_str())) == nullptr) {
                                        throw std::runtime_error("Error while performing GET request");
                                    }
                                }
                                else if (method == RequestMethod::POST) {
                                    if ((response = _cli.Post(path.c_str(), fmt::format(_bodyFmtTemplate, param), "application/x-www-form-urlencoded")) == nullptr) {
                                        throw std::runtime_error("Error while performing POST request");
                                    }
                                }
                                
                                return response;
                            };
                            
                            auto start = std::chrono::steady_clock::now();
                            auto res = ExecuteRequest(_requestMethod, _apiPath);
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
                            
                            if (res != nullptr) {
                                spdlog::trace("Got reponse with status: {} and body {}", res->status, res->body);
                            }
                            
                            samples.push_back(duration.count() / 1000.0);
                        }
                        
                        spdlog::debug("Processing of input: {} finished", param);
                        
                    } catch(const std::runtime_error& e) {
                        
                        spdlog::critical(e.what());
                    } catch(...) {
                        
                        spdlog::critical("Unhandled exception while processing input: {}", param);
                    }
                    
                    _sem.notify();
                    
                    return {param, samples};
                });
                
                _futures.push_back(std::move(task));
            }
            
            // Get results from worker tasks
            for (auto& future : _futures) {
                _results.push_back(future.get());
            }
            _futures.clear();
        }
        
        /**
         * Display the result all worker tasks
         */
        void DisplayResult() const noexcept {
            
            spdlog::info("");
            
            // Create a local copy of results
            auto results = _results;
            
            // Sort results based on duration
            std::sort(results.begin(), results.end(), [this](const auto& a, const auto& b) {
                return _resultFunction(a.samples) < _resultFunction(a.samples);
            });
            
            for (const auto& result : results) {
                
                auto duration = _resultFunction(result.samples);
                spdlog::info("Average time {:.5f}s for input: \"{}\" after {} tries", duration, result.input, result.samples.size());
            }
            spdlog::info("");
        }
        
    public: // Setters
        
        /**
         * Sets which REST request method to use
         */
        void SetRequestType(const RequestMethod method) noexcept { _requestMethod = method; }
        
        /**
         * Sets the API path to use
         */
        void SetAPIPath(const std::string& path) noexcept { _apiPath = path; }
        
        /**
         * Sets the function to use to calculate the final resulting duration
         */
        void SetResultFunc(const ResultFunction& func) noexcept { _resultFunction = func; }
        
        /**
         * Sets the number of samples to perform for each input
         */
        void SetSampleCount(const int count) noexcept { _sampleCount = count; }
        
        /**
         * Sets the string to pass as body in POST requests.
         * {} will be substituted with your test input.
         * e.g. "password={}"
         */
        void SetBodyFormatTemplate(const std::string& fmt) noexcept { _bodyFmtTemplate = fmt; }
        
        
    private:
        
        httplib::Client _cli;
        Semaphore _sem;
        
        std::vector<std::future<WorkerTaskResult>> _futures;
        std::vector<WorkerTaskResult> _results;

        ResultFunction _resultFunction = result::Average;
        
        RequestMethod _requestMethod = RequestMethod::GET;
        
        std::string _apiPath = "";
        std::string _bodyFmtTemplate = "";
        
        int _sampleCount = kDefaultSampleCount;
    };
}
