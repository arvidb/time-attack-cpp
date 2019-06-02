#include "common.h"
#include "worker.h"

#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>

int main() {
    
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    
    spdlog::set_pattern("[%H:%M:%S.%e] %^[%l]%$ %v");
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("Starting Time Attack Cpp");
    
    // Run example
    {
        timeattack::Worker worker("localhost", 8000, 1);
        worker.SetAPIPath("/");
        worker.SetRequestType(timeattack::RequestMethod::POST);
        worker.SetSampleCount(5);
        worker.SetBodyFormatTemplate("password={}");
        worker.SetResultFunc(timeattack::result::Max);
        
        const auto inputCount = 100;
        std::vector<std::string> inputs;
        inputs.reserve(inputCount);
        for (int i=0; i < inputCount; i++) {
            inputs.push_back(fmt::format("{:03d}111222333", i));
        }
        worker.DoWork(inputs);
        
        worker.DisplayResult();
    }
    
    spdlog::info("Finished");
    
    exit(EXIT_SUCCESS);
}