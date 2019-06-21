#include "common.h"
#include "worker.h"

#include <CLI/CLI.hpp>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>

constexpr int kDefaultSampleCount = 5;
constexpr int kDefaultConcurrentJobs = 1;
constexpr int kDefaultPort = 80;
constexpr int kDefaultTimeout = 5;

int main(int argc, char** argv) {
    
    // Parse command line
    
    CLI::App app{"Time Attack Cpp"};
    
    std::string hostname = "localhost";
    app.add_option("-u,--url", hostname, "Target URL/hostname")->required();
    
    int port = kDefaultPort;
    app.add_option("-p,--port", port, "Target port");
    
    std::string endpoint = "/";
    app.add_option("-e,--endpoint", endpoint, "Endpoint");
    
    std::string bodyFormat = "{}";
    app.add_option("-b,--body", bodyFormat, "Body format template");
    
    int timeout = kDefaultTimeout;
    app.add_option("-t,--timeout", timeout, "Timeout value (in seconds) for requests");
    
    int sampleCount = kDefaultSampleCount;
    app.add_option("-s,--samples", sampleCount, "Number of samples to take for each input");
    
    int jobCount = kDefaultConcurrentJobs;
    app.add_option("-j,--jobs", jobCount, "Number of parallel jobs to run for the experiment");
    
    std::vector<std::string> inputs;
    app.add_option("-i,--inputs", inputs, "A list of inputs to test against")->required();
    
    bool verbose;
    app.add_flag("--verbose", verbose);
    
    CLI11_PARSE(app, argc, argv);
    
    
    // Setup Logging
    {
        auto console = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(console);
        
        spdlog::set_pattern("[%H:%M:%S.%e] %^[%l]%$ %v");
        spdlog::set_level(verbose ? spdlog::level::trace : spdlog::level::info);
    }
    
    spdlog::info("Starting Time Attack Cpp");
    
    // Run experiment
    {
        timeattack::Worker worker(hostname, port, timeout, jobCount);
        worker.SetEndpoint(endpoint);
        worker.SetRequestType(timeattack::RequestMethod::POST);
        worker.SetSampleCount(sampleCount);
        worker.SetBodyFormatTemplate(bodyFormat);
        worker.SetResultFunc(timeattack::result::Median);
        
        worker.DoWork(inputs);
        
        worker.DisplayResult();
    }
    
    spdlog::info("Finished");
    
    exit(EXIT_SUCCESS);
}