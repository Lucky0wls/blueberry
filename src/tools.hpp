#pragma once

#include <chrono>
#include <cstdint>

inline std::uint64_t elapsedTime(std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

    return static_cast<std::uint64_t>(elapsed.count());
}

inline std::uint64_t calculateNps(std::uint64_t nodes, std::uint64_t time_ms) {
    if (time_ms == 0) {
        return nodes * 1000;
    }

    return nodes * 1000 / time_ms;
}