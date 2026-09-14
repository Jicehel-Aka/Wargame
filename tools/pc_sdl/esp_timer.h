#pragma once
#include <chrono>
inline int64_t esp_timer_get_time() {
    using namespace std::chrono;
    return (int64_t)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
