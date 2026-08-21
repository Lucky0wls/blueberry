#include <iostream>
#include <chrono>

#include "board.hpp"
#include "movegen.hpp"
#include "uci.hpp"
#include "zobrist.hpp"
#include "search.hpp"

std::uint64_t perft(Board& board, int depth) {
    Color us = board.sideToMove;
    if (depth == 0) {
        return 1;
    }

    moveList moves;
    generatePseudoLegalMoves(board, moves);

    std::uint64_t nodes = 0;

    for (const Move& move : moves) {
        board.makeMove(move);
        if (board.inCheck(us)) {
            board.unmakeMove(move);
            continue;
        }

        nodes += perft(board, depth - 1);

        board.unmakeMove(move);
    }

    return nodes;
}

int main(int argc, char** argv) {
    initZobrist();
    clearTT();

    Board board;

    if (argc >= 2 && std::string(argv[1]) == "bench") {
        bench(board);
        return 0;
    }

    uciLoop(board);

    return 0;
}