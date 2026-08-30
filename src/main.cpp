#include "uci.hpp"

int main(int argc, char** argv) {
    clearTT();
    clearKillers();
    clearHistory();

    Board board(startpos);
    board.set960(false);
    frc = false;

    if (argc >= 2 && std::string(argv[1]) == "bench") {
        bench(board);
        return 0;
    }

    uciLoop(board);

    return 0;
}