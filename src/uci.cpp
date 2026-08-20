#include <sstream>
#include <iostream>
#include <chrono>

#include "uci.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "search.hpp"
#include "types.hpp"

Move bestMove = {e2, e4};
bool haveBestMove = false;

#ifndef GIT_SHA
#define GIT_SHA "nogit"
#endif

#ifndef GIT_DATE
#define GIT_DATE "01011970"
#endif

std::uint64_t elapsedTime(std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - start
    );

    return static_cast<std::uint64_t>(elapsed.count());
}

std::uint64_t calculateNps(std::uint64_t nodes, std::uint64_t time_ms) {
    if (time_ms == 0) {
        return nodes * 1000;
    }

    return nodes * 1000 / time_ms;
}

std::string squareToString(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);

    return std::string{file, rank};
}

Square stringToSquare(const std::string& s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';

    return static_cast<Square>(rank * 8 + file);
}

Move uciToMove(const std::string& uci, const Board& board) {
    Square from = stringToSquare(uci.substr(0, 2));
    Square to = stringToSquare(uci.substr(2, 2));

    moveList moves;
    generatePseudoLegalMoves(board, moves);

    for (const Move& move : moves) {
        if (moveToUci(move) == uci) {
            return move;
        }
    }

    return {NO_SQUARE, NO_SQUARE, QUIET};
}

std::string moveToUci(const Move& move) {
    std::string moveStr = squareToString(move.from) + squareToString(move.to);
    if (move.flag == PROMOTION_BISHOP) moveStr += "b";
    else if (move.flag == PROMOTION_KNIGHT) moveStr += "n";
    else if (move.flag == PROMOTION_ROOK) moveStr += "r";
    else if (move.flag == PROMOTION_QUEEN) moveStr += "q";
    return moveStr;
}

int iterativeDeepening(Board& board, int maxDepth, SearchInfo& info) {
    int bestScore = 0;
    haveBestMove = false;

    for (int depth = 1; depth <= maxDepth; depth++) {
        info.selDepth = 0;
        int score = negamax(board, depth, 0, info);

        if (info.stop) {
            break;
        }

        bestScore = score;
        bestMove = pvTable[0][0];
        haveBestMove = true;

        std::uint64_t time = elapsedTime(info.start);
        std::uint64_t nps = calculateNps(info.nodes, time);
        int mateScore = 0;

        if (std::abs(bestScore) >= 90000 && std::abs(bestScore) <= 110000) {
            if (bestScore > 0) {
                mateScore = int((100000 - bestScore + 1) / 2);
            } else {
                mateScore = int((-100000 - bestScore - 1) / 2);
            }
        }

        if (mateScore != 0) {
            std::cout << "info depth " << depth << " seldepth " << info.selDepth << " nodes " << info.nodes << " nps " << nps << " time " << time << " score mate " << mateScore << " pv ";
            for (int i = 0; i < pvLength[0]; i++) {
                std::cout << moveToUci(pvTable[0][i]) << " ";
            }
            std::cout << "\n";

            break;
        } else {
            std::cout << "info depth " << depth << " seldepth " << info.selDepth << " nodes " << info.nodes << " nps " << nps << " time " << time << " score cp " << bestScore << " pv ";
            for (int i = 0; i < pvLength[0]; i++) {
                std::cout << moveToUci(pvTable[0][i]) << " ";
            }
            std::cout << "\n";
        }
    }

    return bestScore;
}

void bench(Board& board) {
    SearchInfo info {
        .start = std::chrono::steady_clock::now(),
        .timeLimit = 99999999999,
        .stop = false,
        .selDepth = 0,
        .nodes = 0
    };

    int score = iterativeDeepening(board, 5, info);

    std::uint64_t time = elapsedTime(info.start);
    std::uint64_t nps = calculateNps(info.nodes, time);        

    std::cout << info.nodes << " nodes " << nps << " nps\n";
}

void uciLoop(Board& board) {
    std::cout << "Blueberry " << GIT_DATE << "-" << GIT_SHA << " by Lucky0wls\n";

    std::string input;

    while (std::getline(std::cin, input)) {
        std::stringstream ss(input);

        std::string cmd;
        ss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name Blueberry " << GIT_DATE << "-" << GIT_SHA << "\n";
            std::cout << "id author Lucky0wls\n";
            std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            std::cout << "option name Hash type spin default 1 min 1 max 1\n";
            std::cout << "uciok\n";
        } else if (cmd == "isready") {
            std::cout << "readyok\n";
        } else if (cmd == "ucinewgame") {
            board.setStartpos();
        }
        else if (cmd == "position") {
            std::string type;
            ss >> type;

            if (type == "startpos") {
                board.setStartpos();
            }
            else if (type == "fen") {
                std::string fenStr;
                std::string part;

                for (int i = 0; i < 6; ++i) {
                    if (ss >> part) {
                        fenStr += part;

                        if (i != 5) {
                            fenStr += " ";
                        }
                    }
                }

                board.setFen(fenStr);
            }

            std::size_t movesPos = input.find("moves ");

            if (movesPos != std::string::npos) {
                std::stringstream moves_ss(input.substr(movesPos + 6));

                std::string moveStr;

                while (moves_ss >> moveStr) {
                    Move move = uciToMove(moveStr, board);
                    board.makeMove(move);
                }
            }
        } else if (cmd == "go") {
            std::string token;
            int depth = 100;
            bool depthLimit = false;
            std::uint64_t wtime = 600000;
            std::uint64_t winc = 1000;
            std::uint64_t btime = 600000;
            std::uint64_t binc = 1000;
            std::uint64_t timeLimit = 1000000000;

            while (ss >> token) {
                if (token == "depth") {
                    ss >> depth;
                    depthLimit = true;
                } else if (token == "wtime") {
                    ss >> wtime;
                } else if (token == "btime") {
                    ss >> btime;
                } else if (token == "winc") {
                    ss >> winc;
                } else if (token == "binc") {
                    ss >> binc;
                }
            }

            timeLimit = board.sideToMove == WHITE ? (wtime / 20) + (winc / 2) : (btime / 20) + (binc / 2);

            SearchInfo info {
                .start = std::chrono::steady_clock::now(),
                .timeLimit = timeLimit,
                .stop = false,
                .selDepth = 0,
                .nodes = 0
            };

            int score = iterativeDeepening(board, depth, info);

            std::cout << "bestmove " << moveToUci(bestMove) << "\n";
        } else if (cmd == "quit") {
            return;
        }
    }
}