#pragma once
#include <array>
#include <cassert>

#include "types.hpp"
#include "board.hpp"

class moveList {
public:
    void push(const Move& move) {
        assert(count < 256);
        moves[count++] = move;
    }

    int size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    auto begin() {
        return moves.begin();
    }

    auto end() {
        return moves.begin() + count;
    }

    auto begin() const {
        return moves.begin();
    }

    auto end() const {
        return moves.begin() + count;
    }

    Move& operator[](int index) {
        return moves[index];
    }

    const Move& operator[](int index) const {
        return moves[index];
    }

    void clear() {
        count = 0;
    }

private:
    std::array<Move, 256> moves;
    int count = 0;
};

void generatePseudoLegalMoves(const Board& board, moveList& moves);