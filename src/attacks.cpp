#include "bitboard.hpp"
#include "board.hpp"
#include "attacks.hpp"

Bitboard bishopAttacks(int sq, Bitboard occupied) {
    Bitboard blockers = occupied & attacks::BISHOP_MASKS[sq];

    std::uint64_t index = (blockers * attacks::BISHOP_MAGICS[sq]) >> attacks::BISHOP_SHIFTS[sq];

    return attacks::BISHOP_ATTACKS[sq][index];
}

Bitboard rookAttacks(int sq, Bitboard occupied) {
    Bitboard blockers = occupied & attacks::ROOK_MASKS[sq];

    std::uint64_t index = (blockers * attacks::ROOK_MAGICS[sq]) >> attacks::ROOK_SHIFTS[sq];

    return attacks::ROOK_ATTACKS[sq][index];
}