#pragma once

#include <array>

#include "chess.hpp"
#include "tools.hpp"

using namespace chess;

struct searchInfo {
    std::chrono::steady_clock::time_point start;
    std::uint64_t timeLimit = 1000;
    bool stop = false;
    int selDepth = 0;
    std::uint64_t nodes = 0;
};

extern std::array<std::array<Move, 256>, 256> pvTable;
extern std::array<int, 256> pvLength;

int negamax(Board& board, int depth, int alpha, int beta, int ply, searchInfo& info);