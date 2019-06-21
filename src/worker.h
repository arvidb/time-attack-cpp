#pragma once

#include "common.h"
#include "result_functions.h"
#include "rest_client_adapter.h"

#include <chrono>
#include <numeric>
#include <algorithm>

#include <thread>
#include <future>

namespace timeattack
{
    using resultFunc_t = std::function<const duration_t(std::vector<duration_t>)>;
            
    class Worker {
        
        struct WorkerTaskResult {
            std::string input;
            std::vector<duration_t> samples;
        };
        
    public:
        Worker(const std::string& host, const int port, const int timeout, const int maxConcurrentRequests)
            : _cli(host, port, timeout),
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
            
            if (_endpoint.empty()) {
                
                spdlog::error("Endpoint cannot be empty, specify a endpoint starting with a leading slash \"/\"");
                return;
            }
            
            spdlog::info("Starting worker");
            spdlog::info("Body template: {}", _bodyFmtTemplate);
            spdlog::info("Endpoint: {}", _endpoint);
            spdlog::info("Request method: {}",
                          _requestMethod == RequestMethod::GET ? "GET" : "POST");
            
            _results.clear();
            
            CreateTasks(params);
            
            InitProgress(_futures.size() * _sampleCount);
            
            // Get results from worker tasks
            for (auto& future : _futures) {
                _results.push_back(future.get());
            }
            _futures.clear();
        }
        
        void InitProgress(int total) noexcept {
        
            _totalItems = total;
            _processedItems = 0;
            _lastProgress = 0;
        }
        
        void IncreaseProgress() noexcept {
        
            std::unique_lock<std::mutex> lock(_mtxProgress);
            _processedItems++;
            
            auto percentageLeft = static_cast<int>(100 * (static_cast<float>(_processedItems) / _totalItems));
            if (_lastProgress != percentageLeft && percentageLeft % 10 == 0) {
                
                spdlog::info("Progress {}% ({}/{})", percentageLeft, _processedItems, _totalItems);
                _lastProgress = percentageLeft;
            }
        }
        
        /**
         * Display the result all worker tasks
         */
        void DisplayResult() const noexcept {
            
            spdlog::info("");
            
            // Create a local copy of results
            auto results = _results;
            
            // Sort results on duration
            std::sort(results.begin(), results.end(), [this](const auto& a, const auto& b) {
                return _resultFunction(a.samples) < _resultFunction(b.samples);
            });
            
            for (const auto& result : results) {
                
                auto duration = _resultFunction(result.samples);
                spdlog::info("Average time {:.5f}s for input: \"{}\" after {} tries", duration, result.input, result.samples.size());
            }
            spdlog::info("");
        }
        
    private:
        
        void CreateTasks(const std::vector<std::string>& params) {
            
            for (const auto& param : params) {
                
                spdlog::debug("Creating task for input: {}", param);
                
                auto task = std::async([this, param]() -> WorkerTaskResult {
                    
                    _sem.wait();
                    
                    spdlog::debug("Processing input: {} [samples: {}]", param, _sampleCount);
                    
                    std::vector<duration_t> samples;
                    
                    try {
                        
                        for (int i=0; i < _sampleCount; i++) {
                            
                            auto start = std::chrono::steady_clock::now();
                            const auto& res = _cli.ExecuteRequest(_requestMethod, _endpoint, fmt::format(_bodyFmtTemplate, param));
                            auto end = std::chrono::steady_clock::now();
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                            
                            if (res != nullptr) {
                                spdlog::trace("Got reponse with status: {} and body {}", res->status, res->body);
                            }
                            
                            samples.push_back(duration.count() / 1000.0);
                            IncreaseProgress();
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
        }
        
    public: // Setters
        
        /**
         * Sets which REST request method to use
         */
        void SetRequestType(const RequestMethod method) noexcept { _requestMethod = method; }
        
        /**
         * Sets the Endpoint to use
         */
        void SetEndpoint(const std::string& endpoint) noexcept { _endpoint = endpoint; }
        
        /**
         * Sets the function to use to calculate the final resulting duration
         */
        void SetResultFunc(const resultFunc_t& func) noexcept { _resultFunction = func; }
        
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
        
        RestClientAdapter _cli;
        Semaphore _sem;
        
        std::vector<std::future<WorkerTaskResult>> _futures;
        std::vector<WorkerTaskResult> _results;

        resultFunc_t _resultFunction = result::Average;
        
        RequestMethod _requestMethod = RequestMethod::GET;
        
        std::string _endpoint = "";
        std::string _bodyFmtTemplate = "";
        
        int _sampleCount;
        
        int _lastProgress;
        int _totalItems;
        int _processedItems;
        std::mutex _mtxProgress;
    };
}
