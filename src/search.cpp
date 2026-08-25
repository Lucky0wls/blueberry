#include "search.hpp"
#include "evaluate.hpp"
#include "movepicker.hpp"

int inf = 1'000'000'000;
int mateScore = 100'000;

std::array<std::array<Move, 256>, 256> pvTable;
std::array<int, 256> pvLength;

bool timeUp(const searchInfo& info) {
    return elapsedTime(info.start) >= info.timeLimit;
}

int negamax(Board& board, int depth, int alpha, int beta, int ply, searchInfo& info) {
    int bestScore = -inf;
    
    pvLength[ply] = ply;

    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    if ((info.nodes & 2047) == 0 && timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        return 0;
    }

    if (ply > 0) {
        if (board.isHalfMoveDraw()) {
            return board.getHalfMoveDrawType().first == GameResultReason::CHECKMATE ? -mateScore + ply : 0;
        }
        if (board.isRepetition(1)) {
            return 0;
        }
    }

    if (depth < 0) {
        depth = 0;
    }

    alpha = std::max(alpha, -mateScore + ply);
    beta = std::min(beta, mateScore - ply);

    if (alpha >= beta) {
        return alpha;
    }

    if (depth == 0) {
        return evaluate(board);
    }

    if (ply >= 245) {
        return evaluate(board);
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.empty()) {
        return board.inCheck() ? -mateScore + ply : 0;
    }

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return 0;
        }

        pickNextMove(board, moves, i);

        const Move& move = moves[i];

        board.makeMove(move);

        int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, info);

        board.unmakeMove(move);

        if (info.stop) {
            return 0;
        }

        if (score > bestScore) {
            bestScore = score;

            pvTable[ply][ply] = move;

            for (int i = ply + 1; i < pvLength[ply + 1];  i++) {
                pvTable[ply][i] = pvTable[ply + 1][i];
            }

            pvLength[ply] = pvLength[ply + 1];
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            return bestScore;
        }
    }

    return bestScore;
}