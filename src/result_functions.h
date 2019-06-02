#pragma once

#include <algorithm>
#include <numeric>

namespace timeattack::result {
    
    const inline double Average(const std::vector<double>& v) {
        if (v.empty()) {
            return 0.0;
        } else {
            return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        }
    };
    
    const inline double Median(const std::vector<double>& v) {
        if (v.empty()) {
            return 0.0;
        } else {
            std::vector<double> v2 = v;
            size_t n = v2.size() / 2;
            std::nth_element(v2.begin(), v2.begin()+n, v2.end());
            return v[n];
        }
    };
    
    const inline double Min(const std::vector<double>& v) {
        const auto& result = std::min_element(v.begin(), v.end());
        return result != v.end() ? *result : 0.0;
    };
    
    const inline double Max(const std::vector<double>& v) {
        const auto& result = std::max_element(v.begin(), v.end());
        return result != v.end() ? *result : 0.0;
    };
}