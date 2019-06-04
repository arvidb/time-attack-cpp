#include "common.h"
#include "worker.h"

#include <CLI/CLI.hpp>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>

int main(int argc, char** argv) {
    
    // Parse command line
    
    CLI::App app{"Time Attack Cpp"};
    
    std::string hostname = "localhost";
    app.add_option("-u,--url", hostname, "Target URL/hostname")->required();
    
    int port = 80;
    app.add_option("-p,--port", port, "Target port");
    
    std::string apiPath = "/";
    app.add_option("--path", apiPath, "API path");
    
    std::string bodyFormat = "password={}";
    app.add_option("-b,--body", bodyFormat, "Body format template");
    
    int sampleCount = 5;
    app.add_option("-s,--samples", sampleCount, "Number of samples to take for each input");
    
    std::vector<std::string> inputs;
    app.add_option("-i,--inputs", inputs, "A list of inputs to test against")->required();
    
    CLI11_PARSE(app, argc, argv);
    
    
    // Setup Logging
    
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    
    spdlog::set_pattern("[%H:%M:%S.%e] %^[%l]%$ %v");
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("Starting Time Attack Cpp");
    
    // Run experiment
    {
        timeattack::Worker worker(hostname, port, 1);
        worker.SetAPIPath(apiPath);
        worker.SetRequestType(timeattack::RequestMethod::POST);
        worker.SetSampleCount(sampleCount);
        worker.SetBodyFormatTemplate(bodyFormat);
        worker.SetResultFunc(timeattack::result::Max);
        
        worker.DoWork(inputs);
        
        worker.DisplayResult();
    }
    
    spdlog::info("Finished");
    
    exit(EXIT_SUCCESS);
}