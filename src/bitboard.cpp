#include <cstdint>
#include <iostream>

#include "bitboard.hpp"

bool isSet(const Bitboard& bb, int square) {
    return bb & (1ULL << square);
}

int popSq(Bitboard& bb) {
    int square = __builtin_ctzll(bb);
    bb &= bb - 1;
    return square;
}

int count(Bitboard bb) {
    return __builtin_popcountll(bb);
}

void printBitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (isSet(bb, square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }

        std::cout << '\n';
    }
}
