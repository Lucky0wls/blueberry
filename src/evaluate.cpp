#include <algorithm>

#include "board.hpp"
#include "types.hpp"
#include "bitboard.hpp"

int reverseSq(const int sq) {
    return sq ^ 56;
}   

const int pawnValue = 100;
const int knightValue = 320;
const int bishopValue = 330;
const int rookValue = 500;
const int queenValue = 900;

const int pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

const int bishopTable[64] {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

const int rookTable[64] {
    0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int queenTable[64] {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -10, 5, 5, 5, 5, 5, 0, -10,
    0, 0, 5, 5, 5, 5, 0, -5,
    -5, 0, 5, 5, 5, 5, 0, -5,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

const int kingMgTable[64] {
    20, 30, 10, 0, 0, 10, 30, 20,
    20, 20, 0, 0, 0, 0, 20, 20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

const int kingEgTable[64] {
    -50, -30, -30, -30, -30, -30, -30, -50,
    -30, -30, 0, 0, 0, 0, -30, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -20, -10, 0, 0, -10, -20, -30,
    -50, -40, -30, -20, -20, -30, -40, -50
};

int evaluate(const Board& board) {
    int score = 0;

    int wP = count(board.pieces[WHITE][PAWN]);
    int wN = count(board.pieces[WHITE][KNIGHT]);
    int wB = count(board.pieces[WHITE][BISHOP]);
    int wR = count(board.pieces[WHITE][ROOK]);
    int wQ = count(board.pieces[WHITE][QUEEN]);

    int bP = count(board.pieces[BLACK][PAWN]);
    int bN = count(board.pieces[BLACK][KNIGHT]);
    int bB = count(board.pieces[BLACK][BISHOP]);
    int bR = count(board.pieces[BLACK][ROOK]);
    int bQ = count(board.pieces[BLACK][QUEEN]);

    score += pawnValue * wP;
    score -= pawnValue * bP;

    score += knightValue * wN;
    score -= knightValue * bN;

    score += bishopValue * wB;
    score -= bishopValue * bB;
    
    score += rookValue * wR;
    score -= rookValue * bR;

    score += queenValue * wQ;
    score -= queenValue * bQ;

    Bitboard wP_bb = board.pieces[WHITE][PAWN];
    Bitboard wN_bb = board.pieces[WHITE][KNIGHT];
    Bitboard wB_bb = board.pieces[WHITE][BISHOP];
    Bitboard wR_bb = board.pieces[WHITE][ROOK];
    Bitboard wQ_bb = board.pieces[WHITE][QUEEN];
    Bitboard wK_bb = board.pieces[WHITE][KING];

    Bitboard bP_bb = board.pieces[BLACK][PAWN];
    Bitboard bN_bb = board.pieces[BLACK][KNIGHT];
    Bitboard bB_bb = board.pieces[BLACK][BISHOP];
    Bitboard bR_bb = board.pieces[BLACK][ROOK];
    Bitboard bQ_bb = board.pieces[BLACK][QUEEN];
    Bitboard bK_bb = board.pieces[BLACK][KING];
    
    while (wP_bb) {
        int sq = popSq(wP_bb);

        score += pawnTable[sq];
    }
    while (bP_bb) {
        int sq = popSq(bP_bb);

        score -= pawnTable[reverseSq(sq)];
    }

    while (wN_bb) {
        int sq = popSq(wN_bb);

        score += knightTable[sq];
    }
    while (bN_bb) {
        int sq = popSq(bN_bb);

        score -= knightTable[reverseSq(sq)];
    }

    while (wB_bb) {
        int sq = popSq(wB_bb);
        
        score += bishopTable[sq];
    }
    while (bB_bb) {
        int sq = popSq(bB_bb);

        score -= bishopTable[reverseSq(sq)];
    }

    while (wR_bb) {
        int sq = popSq(wR_bb);

        score += rookTable[sq];
    }
    while (bR_bb) {
        int sq = popSq(bR_bb);

        score -= rookTable[reverseSq(sq)];
    }

    while (wQ_bb) {
        int sq = popSq(wQ_bb);

        score += queenTable[sq];
    }
    while (bQ_bb) {
        int sq = popSq(bQ_bb);

        score -= queenTable[reverseSq(sq)];
    }

    while (wK_bb) {
        int sq = popSq(wK_bb);

        if (wQ + bQ == 0) score += kingEgTable[sq];
        else score += kingMgTable[sq];
    }
    while (bK_bb) {
        int sq = popSq(bK_bb);

        if (wQ + bQ == 0) score -= kingEgTable[reverseSq(sq)];
        else score -= kingMgTable[reverseSq(sq)];
    }

    if (board.sideToMove == WHITE) {
        return score;
    } else {
        return -score;
    }
}