#include <iostream>
#include <cassert>
#include <sstream>

#include "board.hpp"
#include "attacks.hpp"
#include "zobrist.hpp"

Board::Board() {
    setStartpos();
}

Piece Board::pieceAt(int sq) const {
    if (isSet(pieces[WHITE][PAWN], sq)) return wP;
    else if (isSet(pieces[WHITE][KNIGHT], sq)) return wN;
    else if (isSet(pieces[WHITE][BISHOP], sq)) return wB;
    else if (isSet(pieces[WHITE][ROOK], sq)) return wR;
    else if (isSet(pieces[WHITE][QUEEN], sq)) return wQ;
    else if (isSet(pieces[WHITE][KING], sq)) return wK;

    else if (isSet(pieces[BLACK][PAWN], sq)) return bP;
    else if (isSet(pieces[BLACK][KNIGHT], sq)) return bN;
    else if (isSet(pieces[BLACK][BISHOP], sq)) return bB;
    else if (isSet(pieces[BLACK][ROOK], sq)) return bR;
    else if (isSet(pieces[BLACK][QUEEN], sq)) return bQ;
    else if (isSet(pieces[BLACK][KING], sq)) return bK;

    return noPiece;
}

PieceInfo Board::pieceInfoAt(int sq) const {
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            if (isSet(pieces[color][piece], sq)) {
                return {static_cast<Color>(color), static_cast<PieceType>(piece), true};
            }
        }
    }

    return {WHITE, PAWN, false};
}

void Board::setPiece(Color color, PieceType piece, int square) {
    pieces[color][piece] |= 1ULL << square;
}

void Board::removePiece(Color color, PieceType piece, int square) {
    pieces[color][piece] &= ~(1ULL << square);
}

Bitboard Board::colorPieces(Color color) const {
    return pieces[color][PAWN] | pieces[color][KNIGHT] | pieces[color][BISHOP] | pieces[color][ROOK] | pieces[color][QUEEN] | pieces[color][KING];
}

Bitboard Board::occupied() const {
    return colorPieces(WHITE) | colorPieces(BLACK);
}

bool Board::isSquareAttacked(Square sq, Color byColor) const {
    Bitboard occ = occupied();

    Bitboard pawns = pieces[byColor][PAWN];
    Bitboard knights = pieces[byColor][KNIGHT];
    Bitboard bishops = pieces[byColor][BISHOP];
    Bitboard rooks = pieces[byColor][ROOK];
    Bitboard queens = pieces[byColor][QUEEN];
    Bitboard king = pieces[byColor][KING];

    Color oppositeColor = byColor == WHITE ? BLACK : WHITE;

    if (attacks::PAWN_ATTACKS[oppositeColor][sq] & pawns) {
        return true;
    }

    if (attacks::KNIGHT_ATTACKS[sq] & knights) {
        return true;
    }

    if (attacks::KING_ATTACKS[sq] & king) {
        return true;
    }

    Bitboard bishopLike = bishopAttacks(sq, occ);

    if (bishopLike & (bishops | queens)) {
        return true;
    }

    Bitboard rookLike = rookAttacks(sq, occ);

    if (rookLike & (rooks | queens)) {
        return true;
    }

    return false;
}

int Board::castlingIndex() const {
    const BoardState& state = stateStack[ply];

    int index = 0;

    if (state.whiteKingsideCastling)  index |= 1;
    if (state.whiteQueensideCastling) index |= 2;
    if (state.blackKingsideCastling)  index |= 4;
    if (state.blackQueensideCastling) index |= 8;

    return index;
}

std::uint64_t Board::computeZobrist() const {
    std::uint64_t key = 0;

    for (int color = 0; color < 2; ++color) {
        for (int piece = 0; piece < 6; ++piece) {
            Bitboard bb = pieces[color][piece];

            while (bb) {
                int sq = popSq(bb);

                key ^= zobristPieces[color][piece][sq];
            }
        }
    }

    key ^= zobristCastling[castlingIndex()];

    if (stateStack[ply].enPassantSquare != NO_SQUARE) {
        int file = stateStack[ply].enPassantSquare % 8;
        key ^= zobristEnPassant[file];
    }

    if (sideToMove == BLACK) {
        key ^= zobristSide;
    }

    return key;
}

bool Board::inCheck(Color color) const {
    Color opponent = color == WHITE ? BLACK : WHITE;
    int kingSq = __builtin_ctzll(pieces[color][KING]);
    return isSquareAttacked(static_cast<Square>(kingSq), opponent);
}

void Board::setStartpos() {
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            pieces[color][piece] = 0;
        }
    }
    pieces[WHITE][PAWN] = (1ULL << a2) | (1ULL << b2) | (1ULL << c2) | (1ULL << d2) | (1ULL << e2) | (1ULL << f2) | (1ULL << g2) | (1ULL << h2);

    pieces[WHITE][KNIGHT] = (1ULL << b1) | (1ULL << g1);

    pieces[WHITE][BISHOP] = (1ULL << c1) | (1ULL << f1);

    pieces[WHITE][ROOK] = (1ULL << a1) | (1ULL << h1);

    pieces[WHITE][QUEEN] = 1ULL << d1;
    pieces[WHITE][KING]  = 1ULL << e1;

    pieces[BLACK][PAWN] = (1ULL << a7) | (1ULL << b7) | (1ULL << c7) | (1ULL << d7) | (1ULL << e7) | (1ULL << f7) | (1ULL << g7) | (1ULL << h7);

    pieces[BLACK][KNIGHT] = (1ULL << b8) | (1ULL << g8);

    pieces[BLACK][BISHOP] = (1ULL << c8) | (1ULL << f8);

    pieces[BLACK][ROOK] = (1ULL << a8) | (1ULL << h8);

    pieces[BLACK][QUEEN] = 1ULL << d8;
    pieces[BLACK][KING]  = 1ULL << e8;

    sideToMove = WHITE;

    ply = 0;

    stateStack[0].whiteKingsideCastling = true;
    stateStack[0].whiteQueensideCastling = true;
    stateStack[0].blackKingsideCastling = true;
    stateStack[0].blackQueensideCastling = true;

    stateStack[0].enPassantSquare = NO_SQUARE;

    stateStack[0].halfmoveClock = 0;
    stateStack[0].fullmoveNumber = 1;

    stateStack[0].captured = {WHITE, PAWN, false};

    stateStack[0].zobristKey = computeZobrist();
}

bool Board::isCapture(const Move& move) const {
    Bitboard them = this->colorPieces(this->sideToMove == WHITE ? BLACK : WHITE);

    return isSet(them, move.to) || move.flag == EN_PASSANT;
}

bool Board::hasNonPawnMaterial(const Color& color) {
    Bitboard nonPawnMaterial = pieces[color][KNIGHT] | pieces[color][BISHOP] | pieces[color][ROOK] | pieces[color][QUEEN];

    return nonPawnMaterial != 0;
}

void Board::makeMove(const Move& move) {
    PieceInfo info = pieceInfoAt(move.from);
    if (info.exists == false) {
        return;
    }

    ply++;
    std::uint64_t& key = stateStack[ply].zobristKey;

    stateStack[ply] = stateStack[ply - 1];
    if (info.color == BLACK) {
        stateStack[ply].fullmoveNumber++;
    }

    if (stateStack[ply].enPassantSquare != NO_SQUARE) {
        int epFile = stateStack[ply].enPassantSquare % 8;
        key ^= zobristEnPassant[epFile];
    }
    key ^= zobristCastling[castlingIndex()];
    
    if (info.type == KING) {
        if (info.color == WHITE) {
            stateStack[ply].whiteKingsideCastling = false;
            stateStack[ply].whiteQueensideCastling = false;
        } else {
            stateStack[ply].blackKingsideCastling = false;
            stateStack[ply].blackQueensideCastling = false;
        }
    }

    if (info.type == ROOK) {
        if (info.color == WHITE) {
            if (move.from == h1) stateStack[ply].whiteKingsideCastling = false;
            if (move.from == a1) stateStack[ply].whiteQueensideCastling = false;
        } else {
            if (move.from == h8) stateStack[ply].blackKingsideCastling = false;
            if (move.from == a8) stateStack[ply].blackQueensideCastling = false;
        }
    }

    int targetSq;

    if (move.flag == EN_PASSANT) {
        targetSq = info.color == WHITE ? move.to - 8 : move.to + 8;
    } else {
        targetSq = move.to;
    }

    PieceInfo target = pieceInfoAt(targetSq);

    stateStack[ply].captured = target;

    if (target.exists) {
        key ^= zobristPieces[target.color][target.type][targetSq];
        removePiece(target.color, target.type, targetSq);
    }

    if (info.type == PAWN || target.exists) {
        stateStack[ply].halfmoveClock = 0;
    } else {
        stateStack[ply].halfmoveClock++;
    }

    if (target.exists && target.type == ROOK) {
        if (target.color == WHITE) {
            if (targetSq == h1) stateStack[ply].whiteKingsideCastling = false;
            if (targetSq == a1) stateStack[ply].whiteQueensideCastling = false;
        } else {
            if (targetSq == h8) stateStack[ply].blackKingsideCastling = false;
            if (targetSq == a8) stateStack[ply].blackQueensideCastling = false;
        }
    }

    if (move.flag == DOUBLE_PAWN_PUSH) {
        if (info.color == WHITE) stateStack[ply].enPassantSquare = static_cast<Square>(move.to - 8);
        else stateStack[ply].enPassantSquare = static_cast<Square>(move.to + 8);
    } else {
        stateStack[ply].enPassantSquare = NO_SQUARE;
    }
    
    if (stateStack[ply].enPassantSquare != NO_SQUARE) {
        int epFile = stateStack[ply].enPassantSquare % 8;
        key ^= zobristEnPassant[epFile];
    }

    key ^= zobristPieces[info.color][info.type][move.from];
    removePiece(info.color, info.type, move.from);  
    
    if (move.flag == PROMOTION_QUEEN) info.type = QUEEN;
    else if (move.flag == PROMOTION_ROOK) info.type = ROOK;
    else if (move.flag == PROMOTION_BISHOP) info.type = BISHOP;
    else if (move.flag == PROMOTION_KNIGHT) info.type = KNIGHT;

    setPiece(info.color, info.type, move.to);
    key ^= zobristPieces[info.color][info.type][move.to];

    if (move.flag == KING_CASTLE) {
        int rookFrom = info.color == WHITE ? h1 : h8;
        int rookTo = info.color == WHITE ? f1 : f8;

        key ^= zobristPieces[info.color][ROOK][rookFrom];
        key ^= zobristPieces[info.color][ROOK][rookTo];

        removePiece(info.color, ROOK, rookFrom);
        setPiece(info.color, ROOK, rookTo);
    }
    if (move.flag == QUEEN_CASTLE) {
        int rookFrom = info.color == WHITE ? a1 : a8;
        int rookTo = info.color == WHITE ? d1 : d8;

        key ^= zobristPieces[info.color][ROOK][rookFrom];
        key ^= zobristPieces[info.color][ROOK][rookTo];

        removePiece(info.color, ROOK, rookFrom);
        setPiece(info.color, ROOK, rookTo);
    }

    key ^= zobristCastling[castlingIndex()];

    sideToMove = sideToMove == WHITE ? BLACK : WHITE;
    key ^= zobristSide;
}

void Board::makeNullMove() {
    ply++;
    std::uint64_t& key = stateStack[ply].zobristKey;

    stateStack[ply] = stateStack[ply - 1];

    if (stateStack[ply].enPassantSquare != NO_SQUARE) {
        key ^= zobristEnPassant[stateStack[ply].enPassantSquare];
        stateStack[ply].enPassantSquare = NO_SQUARE;
    }

    sideToMove = sideToMove == WHITE ? BLACK : WHITE;
    key ^= zobristSide;
}

void Board::unmakeNullMove() {
    sideToMove = sideToMove == WHITE ? BLACK : WHITE;
    ply--;
}

void Board::setFen(const std::string& fen) {
    std::istringstream ss(fen);

    std::string position;
    std::string stm;
    std::string castling;
    std::string ep;

    int halfmove;
    int fullmove;

    ss >> position >> stm >> castling >> ep >> halfmove >> fullmove;

    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            pieces[color][piece] = 0;
        }
    }

    int rank = 7;
    int file = 0;

    for (char c : position) {
        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file += c - '0';
            continue;
        }

        int sq = rank * 8 + file;

        switch (c) {
            case 'P': setPiece(WHITE, PAWN, sq); break;
            case 'N': setPiece(WHITE, KNIGHT, sq); break;
            case 'B': setPiece(WHITE, BISHOP, sq); break;
            case 'R': setPiece(WHITE, ROOK, sq); break;
            case 'Q': setPiece(WHITE, QUEEN, sq); break;
            case 'K': setPiece(WHITE, KING, sq); break;

            case 'p': setPiece(BLACK, PAWN, sq); break;
            case 'n': setPiece(BLACK, KNIGHT, sq); break;
            case 'b': setPiece(BLACK, BISHOP, sq); break;
            case 'r': setPiece(BLACK, ROOK, sq); break;
            case 'q': setPiece(BLACK, QUEEN, sq); break;
            case 'k': setPiece(BLACK, KING, sq); break;
        }

        file++;

        sideToMove = stm == "w" ? WHITE : BLACK;

        ply = 0;

        stateStack[0].whiteKingsideCastling  = castling.find('K') != std::string::npos;
        stateStack[0].whiteQueensideCastling = castling.find('Q') != std::string::npos;
        stateStack[0].blackKingsideCastling  = castling.find('k') != std::string::npos;
        stateStack[0].blackQueensideCastling = castling.find('q') != std::string::npos;

        stateStack[0].halfmoveClock = halfmove;
        stateStack[0].fullmoveNumber = fullmove;

        stateStack[0].captured = {WHITE, PAWN, false};

        if (ep == "-") {
            stateStack[0].enPassantSquare = NO_SQUARE;
        } else {
            int file = ep[0] - 'a';
            int rank = ep[1] - '1';

            stateStack[0].enPassantSquare = static_cast<Square>(rank * 8 + file);
        }
    }

    stateStack[0].zobristKey = computeZobrist();
}

void Board::unmakeMove(const Move& move) {
    PieceInfo info = pieceInfoAt(move.to);

    BoardState& currentState = stateStack[ply];

    int targetSq;

    if (move.flag == EN_PASSANT) {
        targetSq = info.color == WHITE ? move.to - 8 : move.to + 8;
    } else {
        targetSq = move.to;
    }

    PieceType originalType = info.type;
    if (move.flag == PROMOTION_QUEEN || move.flag == PROMOTION_ROOK || move.flag == PROMOTION_BISHOP || move.flag == PROMOTION_KNIGHT) originalType = PAWN;

    removePiece(info.color, info.type, move.to);
    setPiece(info.color, originalType, move.from);

    if (currentState.captured.exists) {
        setPiece(currentState.captured.color, currentState.captured.type, targetSq);
    }

    if (move.flag == KING_CASTLE) {
        int rookFrom = info.color == WHITE ? h1 : h8;
        int rookTo = info.color == WHITE ? f1 : f8;
        removePiece(info.color, ROOK, rookTo);
        setPiece(info.color, ROOK, rookFrom);
    }
    if (move.flag == QUEEN_CASTLE) {
        int rookFrom = info.color == WHITE ? a1 : a8;
        int rookTo = info.color == WHITE ? d1 : d8;
        removePiece(info.color, ROOK, rookTo);
        setPiece(info.color, ROOK, rookFrom);
    }

    sideToMove = sideToMove == WHITE ? BLACK : WHITE;

    ply--;
}

bool Board::isRepetition() const {
    std::uint64_t currentKey = stateStack[ply].zobristKey;

    int earliest = ply - stateStack[ply].halfmoveClock;
    if (earliest < 0) earliest = 0;

    for (int i = ply - 2; i >= earliest; i -= 2) {
        if (stateStack[i].zobristKey == currentKey) {
            return true;
        }
    }

    return false;
}

void Board::print() const {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;

            Piece piece = pieceAt(sq);

            if (piece == wP) std::cout << "P ";
            else if (piece == wN) std::cout << "N ";
            else if (piece == wB) std::cout << "B ";
            else if (piece == wR) std::cout << "R ";
            else if (piece == wQ) std::cout << "Q ";
            else if (piece == wK) std::cout << "K ";
            else if (piece == bP) std::cout << "p ";
            else if (piece == bN) std::cout << "n ";
            else if (piece == bB) std::cout << "b ";
            else if (piece == bR) std::cout << "r ";
            else if (piece == bQ) std::cout << "q ";
            else if (piece == bK) std::cout << "k ";
            else std::cout << ". ";
        }

        std::cout << '\n';
    }
}