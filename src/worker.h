#pragma once

#include "common.h"

#include <cpp-httplib/httplib.h>

#include <chrono>
#include <numeric>
#include <algorithm>

#include <thread>
#include <future>

constexpr int kDefaultSampleCount = 1;
constexpr int kDefaultMaxConcurrentRequests = 5;
constexpr int kDefaultPort = 80;
constexpr int kDefaultTimeout = 5;

struct Worker {
    
    struct WorkerTaskResult {
        std::string input;
        double duration;
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
        
        spdlog::debug("Starting worker [body template: {}]", _bodyFmtTemplate);
        
        _results.clear();
        
        for (const auto& param : params) {
            
            spdlog::debug("Creating task for input: {}", param);
            
            auto ret = std::async([this, param]() -> WorkerTaskResult {
                
                using namespace std::chrono_literals;
                
                _sem.wait();
                
                spdlog::debug("Processing input: {} [samples: {}]", param, _sampleCount);
                
                std::vector<double> samples;
                
                try {
                    
                    for (int i=0; i < _sampleCount; i++) {
                        
                        auto start = std::chrono::steady_clock::now();
                        auto res = _cli.Post("/", fmt::format(_bodyFmtTemplate, param), "application/x-www-form-urlencoded");
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
                        
                        samples.push_back(duration.count() / 1000.0);
                        
                        if (res == nullptr) {
                            spdlog::error("Error while performing POST request");
                        } else {
                            spdlog::trace("Got reponse with status: {} and body {}", res->status, res->body);
                        }
                    }
                    
                    spdlog::debug("Processing of input: {} finished", param);
                
                } catch(...) {
                
                    spdlog::critical("Unhandled exception while processing input: {}", param);
                }
                
                _sem.notify();
                
                return {param, std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size()};
            });
            
            _futures.push_back(std::move(ret));
        }
        
        // Get results from worker tasks
        for (auto& promise : _futures) {
            _results.push_back(promise.get());
        }
        _futures.clear();
        
        // Sort results based on duration
        std::sort(_results.begin(), _results.end(), [](const auto& a, const auto& b) {
            return a.duration < b.duration;
        });
    }
    
    /**
     * Display the result all worker tasks
     */
    void DisplayResult() const noexcept {
        
        for (const auto& result : _results) {
            spdlog::info("Average time {:.5f}s for input: \"{}\"", result.duration, result.input);
        }
    }
    
public: // Setters
    
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
    
    int _sampleCount = kDefaultSampleCount;
    
    std::string _bodyFmtTemplate;
};
