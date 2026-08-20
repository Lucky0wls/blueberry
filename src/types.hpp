#pragma once

enum MoveFlag {
    QUIET,
    CAPTURE,
    DOUBLE_PAWN_PUSH,
    EN_PASSANT,
    KING_CASTLE,
    QUEEN_CASTLE,
    PROMOTION_KNIGHT,
    PROMOTION_BISHOP,
    PROMOTION_ROOK,
    PROMOTION_QUEEN
};

enum Color {
    WHITE = 0,
    BLACK = 1
};

enum PieceType {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum Piece {
    wP,
    wN,
    wB,
    wR,
    wQ,
    wK,
    bP,
    bN,
    bB,
    bR,
    bQ,
    bK,
    noPiece
};

struct PieceInfo {
    Color color;
    PieceType type;
    bool exists;
};

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,

    NO_SQUARE = 64
};

struct Move {
    Square from;
    Square to;
    MoveFlag flag = QUIET;
};