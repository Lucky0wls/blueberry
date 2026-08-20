#pragma once

#include <string>

#include "bitboard.hpp"
#include "types.hpp"

struct BoardState {
    Square enPassantSquare = NO_SQUARE;

    int halfmoveClock = 0;
    int fullmoveNumber = 1;

    bool whiteKingsideCastling = true;
    bool whiteQueensideCastling = true;
    bool blackKingsideCastling = true;
    bool blackQueensideCastling = true;

    PieceInfo captured = {WHITE, PAWN, false};

    std::uint64_t zobristKey = 0;
};

class Board {
public:
    Bitboard pieces[2][6]{};

    Color sideToMove = WHITE;

    BoardState stateStack[256]{};
    int ply = 0;

    Board();

    void setStartpos();

    bool inCheck(Color color) const;

    Piece pieceAt(int sq) const;

    int castlingIndex() const;

    std::uint64_t computeZobrist() const;

    Bitboard colorPieces(Color color) const;
    Bitboard occupied() const;

    bool isSquareAttacked(Square sq, Color byColor) const;

    void setPiece(Color color, PieceType piece, int square);
    void removePiece(Color color, PieceType piece, int square);

    PieceInfo pieceInfoAt(int sq) const;

    void setFen(const std::string& fen);

    void makeMove(const Move& move);
    void unmakeMove(const Move& move);

    bool isRepetition() const;

    void print() const;
};