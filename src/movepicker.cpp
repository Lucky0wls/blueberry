#include "movepicker.hpp"

int pieceValue(PieceType piece) {
    if (piece == PieceType::PAWN) {
        return 100;
    } 
    if (piece == PieceType::KNIGHT) {
        return 300;
    }
    if (piece == PieceType::BISHOP) {
        return 320;
    }
    if (piece == PieceType::ROOK) {
        return 500;
    }
    if (piece == PieceType::QUEEN) {
        return 900;
    }
    if (piece == PieceType::KING) {
        return 20000;
    }
    
    return 0;
}

int mvvLva(const Board& board, const Move& move) {
    Square from = move.from();
    Square to = move.to();

    Piece attacker = board.at(from);
    Piece victim = board.at(to);

    if (victim == Piece::NONE) {
        return 0;
    }

    int victim_value = pieceValue(victim.type());
    int attacker_value = pieceValue(attacker.type());

    return 10'000 + victim_value * 10 - attacker_value;
}

int scoreMove(const Board& board, const Move& hint, const Move& move) {
    std::uint64_t ttKey = board.hash();
    int ttIndex = ttKey & (1048576 - 1);

    if (tt[ttIndex].key == ttKey && tt[ttIndex].bestMove == move) {
        return 2'000'000;
    }

    if (move == hint && hint != Move::NO_MOVE) {
        return 1'000'000;
    }

    return mvvLva(board, move);
}

void pickNextMove(const Board& board, Movelist& moves, int start, const Move& hint) {
    int best = -1'000'000'000;
    int bestIndex = start;

    for (int i = start; i < moves.size(); i++) {
        int score = scoreMove(board, hint, moves[i]);

        if (score > best) {
            best = score;
            bestIndex = i;
        }
    }

    std::swap(moves[start], moves[bestIndex]);
}