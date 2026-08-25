#include "uci.hpp"

int main(int argc, char** argv) {
    Board board(startpos);

    if (argc >= 2 && std::string(argv[1]) == "bench") {
        bench(board);
        return 0;
    }

    uciLoop(board);

    return 0;
}