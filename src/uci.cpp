#include "uci.hpp"

#ifndef GIT_SHA
#define GIT_SHA "nogit"
#endif

#ifndef GIT_DATE
#define GIT_DATE "01011970"
#endif

Move bestMove = Move::NO_MOVE;

std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int iterativeDeepening(Board& board, int maxDepth, searchInfo& info) {
    int bestScore = 0;
    Move hint = Move::NO_MOVE;

    for (int depth = 1; depth <= maxDepth; depth++) {
        info.selDepth = 0;
        if (depth > 1) {
            hint = bestMove;
        }

        int score = negamax(board, depth, -1'000'000'000, 1'000'000'000, 0, info, hint);

        if (info.stop) {
            break;
        }

        bestScore = score;
        bestMove = pvTable[0][0];

        std::uint64_t time = elapsedTime(info.start);
        std::uint64_t nps = calculateNps(info.nodes, time);
        int mateScore = 0;

        if (std::abs(bestScore) >= 99000 && std::abs(bestScore) <= 100000) {
            if (bestScore > 0) {
                mateScore = (100000 - bestScore + 1) / 2;
            } else {
                mateScore = (-100000 - bestScore - 1) / 2;
            }
        }

        if (mateScore != 0) {
            if (info.selDepth == 0) info.selDepth = depth;
            std::cout << "info depth " << depth << " seldepth " << info.selDepth << " nodes " << info.nodes << " nps " << nps << " time " << time << " score mate " << mateScore << " pv ";
            for (int i = 0; i < pvLength[0]; i++) {
                std::cout << uci::moveToUci(pvTable[0][i]) << " ";
            }
        } else {
            if (info.selDepth == 0) info.selDepth = depth;
            std::cout << "info depth " << depth << " seldepth " << info.selDepth << " nodes " << info.nodes << " nps " << nps << " time " << time << " score cp " << bestScore << " pv ";
            for (int i = 0; i < pvLength[0]; i++) {
                std::cout << uci::moveToUci(pvTable[0][i]) << " ";
            }
        }

        std::cout << "\n";

        if (depth >= 245) {
            break;
        }
    }

    return bestScore;
}

void bench(Board& board) {
    searchInfo info {
        .start = std::chrono::steady_clock::now(),
        .timeLimit = 99999999999,
        .stop = false,
        .selDepth = 0,
        .nodes = 0
    };

    int score = iterativeDeepening(board, 8, info);

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
            clearTT();
            board.setFen(startpos);
        } else if (cmd == "position") {
            std::string type;
            ss >> type;

            if (type == "startpos") {
                board.setFen(startpos);
            } else if (type == "fen") {
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
                    Move move = uci::uciToMove(board, moveStr);
                    board.makeMove(move);
                }
            }
        } else if (cmd == "go") {
            std::string token;
            int depth = 245;
            bool depthLimit = false;
            std::uint64_t wtime = 0;
            std::uint64_t winc = 0;
            std::uint64_t btime = 0;
            std::uint64_t binc = 0;
            std::uint64_t timeLimit = 5000;

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

            bool isTimeLimited = wtime + btime + binc + winc != 0;

            if (isTimeLimited) timeLimit = board.sideToMove() == Color::WHITE ? (wtime / 20) + (winc / 2) : (btime / 20) + (binc / 2); else if (depthLimit) timeLimit = 9999999999999;

            searchInfo info {
                .start = std::chrono::steady_clock::now(),
                .timeLimit = timeLimit,
                .stop = false,
                .selDepth = 0,
                .nodes = 0
            };

            int score = iterativeDeepening(board, depth, info);

            std::cout << "bestmove " << uci::moveToUci(bestMove) << "\n";
        } else if (cmd == "quit") {
            return;
        }
    }
}