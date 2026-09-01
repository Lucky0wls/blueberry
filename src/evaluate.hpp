#include "chess.hpp"

using namespace chess;

struct evalState {
    int mgScore = 0;
    int egScore = 0;
    int phase = 0;
};

extern evalState evalStack[256];

void updateEvalState(const Board& board, const Move& move, evalState& state);

evalState makeEvalState(const Board& board);

int evaluate(const Board& board, const evalState& state);