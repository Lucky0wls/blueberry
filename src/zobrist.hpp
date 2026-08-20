#pragma once

#include <cstdint>

#include "types.hpp"

extern std::uint64_t zobristPieces[2][6][64];
extern std::uint64_t zobristCastling[16];
extern std::uint64_t zobristEnPassant[8];
extern std::uint64_t zobristSide;

void initZobrist();