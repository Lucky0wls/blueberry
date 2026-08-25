#pragma once

#include <array>

#include "chess.hpp"
#include "tools.hpp"

using namespace chess;

enum { ttNone, ttAlpha, ttBeta, ttExact };

struct searchInfo {
    std::chrono::steady_clock::time_point start;
    std::uint64_t timeLimit = 1000;
    bool stop = false;
    int selDepth = 0;
    std::uint64_t nodes = 0;
};

struct TTEntry {
    std::uint64_t key = 0;
    Move bestMove = Move::NO_MOVE;
    int score = 0;
    int depth = -1;
    int flag = ttNone;
};

extern TTEntry tt[1048576];

extern std::array<std::array<Move, 256>, 256> pvTable;
extern std::array<int, 256> pvLength;

void clearTT();

int negamax(Board& board, int depth, int alpha, int beta, int ply, searchInfo& info, const Move& hint);