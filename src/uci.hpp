#include <string>

#include "board.hpp"

std::string moveToUci(const Move& move);

void bench(Board& board);

void uciLoop(Board& board);