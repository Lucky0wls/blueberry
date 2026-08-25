#include "search.hpp"
#include "evaluate.hpp"
#include "movepicker.hpp"

int inf = 1'000'000'000;
int mateScore = 100'000;

TTEntry tt[1048576];

void clearTT() {
    for (int i = 0; i < 1048576; i++) {
        tt[i].key = 0;
        tt[i].bestMove = Move::NO_MOVE;
    }
}

int scoreToTT(int score, int ply) {
    if (score > 99000 && score <= 100000)
        return score + ply;

    if (score < -99000 && score >= -100000)
        return score - ply;

    return score;
}

int scoreFromTT(int score, int ply) {
    if (score > 99000 && score <= 100000)
        return score - ply;

    if (score < -99000 && score >= -100000)
        return score + ply;

    return score;
}

void storeTT(const Move& move, std::uint64_t key, int depth, int score, int flag, int ply) {
    int i = key & (1048576 - 1);
    if (move != Move::NO_MOVE) {
        tt[i].key = key;
        tt[i].depth = depth;
        tt[i].flag = flag;
        tt[i].bestMove = move;
        tt[i].score = scoreToTT(score, ply);
    }
}

int probeTT(int index, std::uint64_t key, int depth, int alpha, int beta, int ply) {
    if (tt[index].depth >= depth) {
        int score = scoreFromTT(tt[index].score, ply);

        if (tt[index].flag == ttExact) {
            return score;
        }

        if (tt[index].flag == ttAlpha && score <= alpha) {
            return score;
        }

        if (tt[index].flag == ttBeta && score >= beta) {
            return score;
        }

    }

    return -inf;
}

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

    std::uint64_t ttKey = board.hash();
    int ttIndex = ttKey & (1048576 - 1);

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

    int originalAlpha = alpha;

    if (ttKey == tt[ttIndex].key) {
        int ttScore = probeTT(ttIndex, ttKey, depth, alpha, beta, ply);
        if (ttScore != -inf) {
            pvTable[ply][ply] = tt[ttIndex].bestMove;
            pvLength[ply] = ply + 1;

            return ttScore;
        }
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
            storeTT(move, ttKey, depth, bestScore, ttBeta, ply);
            return bestScore;
        }
    }

    if (alpha != originalAlpha) {
        storeTT(pvTable[ply][ply], ttKey, depth, bestScore, ttExact, ply);
    } else {
        storeTT(pvTable[ply][ply], ttKey, depth, bestScore, ttAlpha, ply);
    }

    return bestScore;
}