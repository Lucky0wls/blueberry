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

struct ScoredMove {
    Move move;
    int score;
};

static std::uint64_t elapsedTime(std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

    return static_cast<std::uint64_t>(elapsed.count());
}

bool timeUp(const SearchInfo& info) {
    return elapsedTime(info.start) >= info.timeLimit;
}

int negamax(Board& board, int depth, int ply, SearchInfo& info, int alpha, int beta, Move hint) {
    pvLength[ply] = ply;

    Color stm = board.sideToMove;
    int bestScore = -inf;
    int moveNumber = 0;
    int legal = 0;
    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    if (timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        int score = evaluate(board);
        return score;
    }

    if (ply > 0 && (board.stateStack[board.ply].halfmoveClock >= 100 || board.isRepetition())) {
        return drawScore;
    }

    if (depth < 0) {
        depth = 0;
    }

    if (depth == 0) {
        int score = evaluate(board);
        return score;
    }

    moveList moves;
    generatePseudoLegalMoves(board, moves);

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return bestScore;
        }

        pickNextMove(board, moves, hint, i);

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

    if (legal == 0) {
        return board.inCheck(stm) ? -mateScore + ply : drawScore;
    }

    return bestScore;
}