#pragma once

#include <mutex>
#include <condition_variable>

// https://stackoverflow.com/a/4793662
class Semaphore {
public:
    Semaphore (int count = 0)
        : _count(count)
    {}
    
    inline void Notify() {
        std::unique_lock<std::mutex> lock(_mtx);
        _count++;
        _cv.notify_one();
    }
    
    inline void Wait() {
        
        std::unique_lock<std::mutex> lock(_mtx);
        _cv.wait(lock, [this]() { return _count > 0; });
        _count--;
    }
    
private:
    std::mutex _mtx;
    std::condition_variable _cv;
    int _count;
};