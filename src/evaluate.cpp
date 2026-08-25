#include "evaluate.hpp"

const int pawnValue = 100;
const int knightValue = 320;
const int bishopValue = 330;
const int rookValue = 500;
const int queenValue = 900;

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

    score += 100 * (wP - bP);
    score += 300 * (wN - bN);
    score += 320 * (wB - bB);
    score += 500 * (wR - bR);
    score += 900 * (wQ - bQ);

    if (board.sideToMove() == Color::WHITE)
        return score;
    else
        return -score;
}