#include <array>
#include <iostream>
#include <algorithm>

#include "search.hpp"
#include "board.hpp"
#include "types.hpp"
#include "evaluate.hpp"
#include "movegen.hpp"
#include "uci.hpp"
#include "movepicker.hpp"

const int inf = 1'000'000'000;
const int mateScore = 100'000;
const int drawScore = 0;

std::array<std::array<Move, 256>, 256> pvTable{};
std::array<int, 256> pvLength{};

TTEntry tt[1048576];

int scoreToTT(int score, int ply) {
    if (score > 90000 && score < 110000)
        return score + ply;

    if (score < -90000 && score > -110000)
        return score - ply;

    return score;
}

int scoreFromTT(int score, int ply) {
    if (score > 90000 && score < 110000)
        return score - ply;

    if (score < -90000 && score > -110000)
        return score + ply;

    return score;
}

void storeTT(const Move& move, std::uint64_t key, int depth, int score, int flag, int ply) {
    int i = key & (1048576 - 1);
    if (move != NO_MOVE) {
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

void clearTT() {
    for (int i = 0; i < 1048576; i++) {
        tt[i].key = 0;
        tt[i].bestMove = NO_MOVE;
    }
}

static std::uint64_t elapsedTime(std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

    return static_cast<std::uint64_t>(elapsed.count());
}

bool timeUp(const SearchInfo& info) {
    return elapsedTime(info.start) >= info.timeLimit;
}

int qsearch(Board& board, int ply, SearchInfo& info, int alpha, int beta) {
    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    int legal = 0;
    Color stm = board.sideToMove;
    bool inCheck = board.inCheck(stm);

    if ((info.nodes & 2047) == 0 && timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        return evaluate(board);
    }

    if (ply > 0 && (board.stateStack[board.ply].halfmoveClock >= 100 || board.isRepetition())) {
        return drawScore;
    }

    moveList moves;

    if (board.inCheck(stm)) {
        generatePseudoLegalMoves(board, moves);
    } else {
        int standPat = evaluate(board);

        if (standPat >= beta) {
            return standPat;
        }

        if (standPat > alpha) {
            alpha = standPat;
        }

        generatePseudoLegalCaptures(board, moves);
    }

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return -inf;
        }

        pickNextMove(board, moves, NO_MOVE, i, -1, -1);

        const Move& move = moves[i];

        board.makeMove(move);

        if (board.inCheck(stm)) {
            board.unmakeMove(move);
            continue;
        }

        legal++;

        int score = -qsearch(board, ply + 1, info, -beta, -alpha);

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

    if (inCheck && legal == 0) {
        return -mateScore + ply;
    }

    return alpha;
}

int negamax(Board& board, int depth, int ply, SearchInfo& info, int alpha, int beta, Move hint) {
    pvLength[ply] = ply;
    Move bestMove = NO_MOVE;
    std::uint64_t key = board.stateStack[board.ply].zobristKey;
    int ttIndex = key & (1048576 - 1);
    int originalAlpha = alpha;
    Color stm = board.sideToMove;
    int bestScore = -inf;
    int moveNumber = 0;
    int legal = 0;
    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    if ((info.nodes & 2047) == 0 && timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        return evaluate(board);
    }

    if (ply > 0 && (board.stateStack[board.ply].halfmoveClock >= 100 || board.isRepetition())) {
        return drawScore;
    }

    if (depth < 0) {
        depth = 0;
    }

    if (depth == 0) {
        return qsearch(board, ply, info, alpha, beta);
    }

    if (key == tt[ttIndex].key) {
        int ttScore = probeTT(ttIndex, key, depth, alpha, beta, ply);
        if (ttScore != -inf) {
            pvTable[ply][ply] = tt[ttIndex].bestMove;
            pvLength[ply] = ply + 1;

            return ttScore;
        }
    }

    moveList moves;
    generatePseudoLegalMoves(board, moves);

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return bestScore;
        }

        pickNextMove(board, moves, hint, i, ttIndex, key);

        const Move& move = moves[i];

        board.makeMove(move);

        if (board.inCheck(stm)) {
            board.unmakeMove(move);
            continue;
        }

        legal++;

        if (ply == 0) {
            moveNumber++;
            std::cout << "info currmove " << moveToUci(move) << " currmovenumber " << moveNumber << "\n";
        }

        int score = -negamax(board, depth - 1, ply + 1, info, -beta, -alpha, {NO_SQUARE, NO_SQUARE});

        board.unmakeMove(move);

        if (info.stop) {
            return bestScore;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

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
            storeTT(move, key, depth, bestScore, ttBeta, ply);
            return bestScore;
        }
    }

    if (legal == 0) {
        return board.inCheck(stm) ? -mateScore + ply : drawScore;
    }

    if (alpha != originalAlpha) {
        storeTT(bestMove, key, depth, bestScore, ttExact, ply);
    } else {
        storeTT(bestMove, key, depth, bestScore, ttAlpha, ply);
    }

    return bestScore;
}