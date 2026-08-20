#include "zobrist.hpp"
#include "board.hpp"

std::uint64_t zobristPieces[2][6][64];
std::uint64_t zobristCastling[16];
std::uint64_t zobristEnPassant[8];
std::uint64_t zobristSide;

std::uint64_t seed = 0x123456789ABCDEF0ULL;

std::uint64_t random64() {
    std::uint64_t z = (seed += 0x9E3779B97F4A7C15ULL);

    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;

    return z ^ (z >> 31);
}

void initZobrist() {
    for (int color = 0; color < 2; ++color) {
        for (int piece = 0; piece < 6; ++piece) {
            for (int sq = 0; sq < 64; ++sq) {
                zobristPieces[color][piece][sq] = random64();
            }
        }
    }

    for (int i = 0; i < 16; ++i) {
        zobristCastling[i] = random64();
    }

    for (int file = 0; file < 8; ++file) {
        zobristEnPassant[file] = random64();
    }

    zobristSide = random64();
}