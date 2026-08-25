#include "evaluate.hpp"

int reverseSq(const int sq) {
    return sq ^ 56;
}

constexpr int pawnValue = 100;
constexpr int knightValue = 320;
constexpr int bishopValue = 330;
constexpr int rookValue = 500;
constexpr int queenValue = 900;

constexpr int pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0
};

constexpr int knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

constexpr int bishopTable[64] {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

constexpr int rookTable[64] {
    0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

constexpr int queenTable[64] {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -10, 5, 5, 5, 5, 5, 0, -10,
    0, 0, 5, 5, 5, 5, 0, -5,
    -5, 0, 5, 5, 5, 5, 0, -5,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

constexpr int kingMgTable[64] {
    20, 30, 10, 0, 0, 10, 30, 20,
    20, 20, 0, 0, 0, 0, 20, 20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

constexpr int kingEgTable[64] {
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

    int wP = board.pieces(PieceType::PAWN, Color::WHITE).count();
    int wN = board.pieces(PieceType::KNIGHT, Color::WHITE).count();
    int wB = board.pieces(PieceType::BISHOP, Color::WHITE).count();
    int wR = board.pieces(PieceType::ROOK, Color::WHITE).count();
    int wQ = board.pieces(PieceType::QUEEN, Color::WHITE).count();

    int bP = board.pieces(PieceType::PAWN, Color::BLACK).count();
    int bN = board.pieces(PieceType::KNIGHT, Color::BLACK).count();
    int bB = board.pieces(PieceType::BISHOP, Color::BLACK).count();
    int bR = board.pieces(PieceType::ROOK, Color::BLACK).count();
    int bQ = board.pieces(PieceType::QUEEN, Color::BLACK).count();

    Bitboard wP_bb = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard wN_bb = board.pieces(PieceType::KNIGHT, Color::WHITE);
    Bitboard wB_bb = board.pieces(PieceType::BISHOP, Color::WHITE);
    Bitboard wR_bb = board.pieces(PieceType::ROOK, Color::WHITE);
    Bitboard wQ_bb = board.pieces(PieceType::QUEEN, Color::WHITE);
    Bitboard wK_bb = board.pieces(PieceType::KING, Color::WHITE);

    Bitboard bP_bb = board.pieces(PieceType::PAWN, Color::BLACK);
    Bitboard bN_bb = board.pieces(PieceType::KNIGHT, Color::BLACK);
    Bitboard bB_bb = board.pieces(PieceType::BISHOP, Color::BLACK);
    Bitboard bR_bb = board.pieces(PieceType::ROOK, Color::BLACK);
    Bitboard bQ_bb = board.pieces(PieceType::QUEEN, Color::BLACK);
    Bitboard bK_bb = board.pieces(PieceType::KING, Color::BLACK);

    score += pawnValue * (wP - bP);
    score += knightValue * (wN - bN);
    score += bishopValue * (wB - bB);
    score += rookValue * (wR - bR);
    score += queenValue * (wQ - bQ);

    while (wP_bb) {
        int sq = wP_bb.pop();

        score += pawnTable[sq];
    }

    while (wN_bb) {
        int sq = wN_bb.pop();

        score += knightTable[sq];
    }

    while (wB_bb) {
        int sq = wB_bb.pop();

        score += bishopTable[sq];
    }

    while (wR_bb) {
        int sq = wR_bb.pop();

        score += rookTable[sq];
    }

    while (wQ_bb) {
        int sq = wQ_bb.pop();

        score += queenTable[sq];
    }

    while (wK_bb) {
        int sq = wK_bb.pop();

        if (wQ + bQ == 0) {
            score += kingEgTable[sq];
        } else {
            score += kingMgTable[sq];
        }
    }

    while (bP_bb) {
        int sq = bP_bb.pop();

        score -= pawnTable[reverseSq(sq)];
    }

    while (bN_bb) {
        int sq = bN_bb.pop();

        score -= knightTable[reverseSq(sq)];
    }

    while (bB_bb) {
        int sq = bB_bb.pop();

        score -= bishopTable[reverseSq(sq)];
    }

    while (bR_bb) {
        int sq = bR_bb.pop();

        score -= rookTable[reverseSq(sq)];
    }

    while (bQ_bb) {
        int sq = bQ_bb.pop();

        score -= queenTable[reverseSq(sq)];
    }

    while (bK_bb) {
        int sq = bK_bb.pop();

        if (wQ + bQ == 0) {
            score -= kingEgTable[reverseSq(sq)];
        } else {
            score -= kingMgTable[reverseSq(sq)];
        }
    }

    if (board.sideToMove() == Color::WHITE)
        return score;
    else
        return -score;
}