#pragma once

#include <chrono>
#include <array>

#include "board.hpp"
#include "types.hpp"
#include "evaluate.hpp"
#include "movegen.hpp"

struct SearchInfo {
    std::chrono::steady_clock::time_point start;
    std::uint64_t timeLimit = 1000;
    bool stop = false;
    int selDepth = 0;
    std::uint64_t nodes = 0;
};

extern std::array<std::array<Move, 256>, 256> pvTable;
extern std::array<int, 256> pvLength;

int negamax(Board& board, int depth, int ply, SearchInfo& info);