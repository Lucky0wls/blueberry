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

int qsearch(Board& board, int ply, int alpha, int beta, searchInfo& info) {
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

    if (ply >= 245) {
        return evaluate(board);
    }

    Movelist moves;

    alpha = std::max(alpha, -mateScore + ply);
    beta = std::min(beta, mateScore - ply);

    if (alpha >= beta) {
        return alpha;
    }

    if (board.inCheck()) {
        movegen::legalmoves(moves, board);
        if (moves.empty()) {
            return -mateScore + ply;
        }
    } else {
        int standPat = evaluate(board);

        if (standPat >= beta) {
            return standPat;
        }

        if (standPat > alpha) {
            alpha = standPat;
        }

        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    }

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return 0;
        }

        pickNextMove(board, moves, i, Move::NO_MOVE);

        const Move& move = moves[i];

        board.makeMove(move);

        int score = -qsearch(board, ply + 1, -beta, -alpha, info);

        board.unmakeMove(move);

        if (info.stop) {
            return 0;
        }

        if (score >= beta) {
            return score;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int negamax(Board& board, int depth, int alpha, int beta, int ply, searchInfo& info, const Move& hint) {
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
        return qsearch(board, ply, alpha, beta, info);
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

        pickNextMove(board, moves, i, hint);

        const Move& move = moves[i];
        
        board.makeMove(move);

        int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, info, Move::NO_MOVE);

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