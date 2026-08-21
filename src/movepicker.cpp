#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"
#include "search.hpp"

int pieceValue(PieceType piece) {
    if (piece == PAWN) {
        return 100;
    } 
    if (piece == KNIGHT) {
        return 300;
    }
    if (piece == BISHOP) {
        return 320;
    }
    if (piece == ROOK) {
        return 500;
    }
    if (piece == QUEEN) {
        return 900;
    }
    if (piece == KING) {
        return 20000;
    }
    
    return 0;
}

int mvvLva(const Board& board, const Move& move) {
    Square from = move.from;
    Square to = move.to;

    PieceInfo attacker = board.pieceInfoAt(from);
    PieceInfo victim = board.pieceInfoAt(to);

    if (!victim.exists) {
        return 0;
    }

    int victim_value = pieceValue(victim.type);
    int attacker_value = pieceValue(attacker.type);

    return 10'000 + victim_value * 10 - attacker_value;
}

int scoreMove(const Board& board, Move move, Move hint, int ttIndex, std::uint64_t ttKey) {
    if (ttIndex >= 0 && ttKey >= 0 && tt[ttIndex].key == ttKey && move == tt[ttIndex].bestMove) {
        return 2'000'000;
    }

    if (hint != NO_MOVE && move == hint) {
        return 1'000'000;
    }

    if (board.isCapture(move)) {
        return 600'000 + mvvLva(board, move);
    }

    return 0;
}

void pickNextMove(const Board& board, moveList& moves, Move hint, int start, int ttIndex, std::uint64_t ttKey) {
    int best = 0;
    int bestIndex = start;

    for (int i = start + 1; i < moves.size(); i++) {
        int score = scoreMove(board, moves[i], hint, ttIndex, ttKey);

        if (score > best) {
            best = score;
            bestIndex = i;
        }
        if (best == 2'000'000) {
            break;
        }
    }

    std::swap(moves[start], moves[bestIndex]);
}