#include "board.hpp"
#include "movegen.hpp"

void pickNextMove(const Board& board, moveList& moves, Move hint, int start, int ttIndex, std::uint64_t ttKey);