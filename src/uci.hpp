#include <iostream>
#include <sstream>

#include "search.hpp"

extern std::string startpos;

extern bool frc;

int iterativeDeepening(Board& board, int maxDepth, searchInfo& info);

void uciLoop(Board& board);