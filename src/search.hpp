#pragma once

#include <chrono>
#include <array>

#include "board.hpp"
#include "types.hpp"
#include "evaluate.hpp"
#include "movegen.hpp"

enum { ttNone, ttAlpha, ttBeta, ttExact };

struct SearchInfo {
    std::chrono::steady_clock::time_point start;
    std::uint64_t timeLimit = 1000;
    bool stop = false;
    int selDepth = 0;
    std::uint64_t nodes = 0;
};

struct TTEntry {
    std::uint64_t key = 0;
    Move bestMove = NO_MOVE;
    int score = 0;
    int depth = -1;
    int flag = ttNone;
};

extern TTEntry tt[1048576];

extern Move killer1[256];
extern Move killer2[256];

void clearTT();
void clear_killers();

extern std::array<std::array<Move, 256>, 256> pvTable;
extern std::array<int, 256> pvLength;

int negamax(Board& board, int depth, int ply, SearchInfo& info, int alpha, int beta, Move hint);