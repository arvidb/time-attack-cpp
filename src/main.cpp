#include "common.h"
#include "worker.h"

#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>

int main() {
    
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    
    spdlog::set_pattern("[%H:%M:%S.%e] %^[%l]%$ %v");
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("Starting TimeAttackCpp");
    
    Worker worker("localhost", 8000, 1);
    worker.SetSampleCount(5);
    worker.SetBodyFormatTemplate("password={}");
    
    std::vector<std::string> inputs;
    inputs.reserve(10);
    for (int i=0; i < 10; i++) {
        inputs.push_back(fmt::format("{:03d}111222333", i));
    }
    worker.DoWork(inputs);
    
    worker.DisplayResult();
    
    spdlog::info("Finished");
    
    exit(EXIT_SUCCESS);
}