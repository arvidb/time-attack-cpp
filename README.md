# Time Attack Cpp
A simple utility I made for a CTF challenge to measures the average response time for a REST endpoint.

# Features
* HTTP POST/GET method
* Use average, median or percentile functions to calculate the result
* Test against multiple (request body) inputs
  * Currently supports using the placeholder "{}" in a request body.
  * The Placeholder tag will be substituted by all the provided inputs.
* Concurrent requests

# Technologies
* cpp-httplib - HTTP library (https://github.com/yhirose/cpp-httplib)
* spdlog - Logging library (https://github.com/gabime/spdlog)
* fmt - Formatting library (https://github.com/fmtlib/fmt)

# Dependencies
* CMake 3.0
* Requires C++17 compatible compiler

# Installation
## Clone
Clone this repository using
```git clone --recursive https://github.com/arvidb/time-attack-cpp.git```


# Build
* Create a build folder ```mkdir build && cd build```
* Build with cmake ```cmake ..```
