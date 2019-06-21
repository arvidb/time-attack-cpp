#pragma once

#include <algorithm>
#include <numeric>

namespace timeattack::result {
    
    const inline duration_t Average(const std::vector<duration_t>& v) {
        if (v.empty()) {
            return 0.0;
        } else {
            return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        }
    }
    
    const inline duration_t Percentile(const std::vector<duration_t>& v, double percent)
    {
        if (v.empty()) {
            return 0.0;
        } else {
            std::vector<duration_t> tmp = v;
            auto nth = tmp.begin() + (percent*tmp.size())/100;
            std::nth_element(tmp.begin(), nth, tmp.end());
            return *nth;
        }
    }
    
    const inline duration_t Median(const std::vector<duration_t>& v) {
        return Percentile(v, 50);
    }
    
    const inline duration_t Min(const std::vector<duration_t>& v) {
        const auto& result = std::min_element(v.begin(), v.end());
        return result != v.end() ? *result : 0.0;
    }
    
    const inline duration_t Max(const std::vector<duration_t>& v) {
        const auto& result = std::max_element(v.begin(), v.end());
        return result != v.end() ? *result : 0.0;
    }
}