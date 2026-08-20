#include "movegen.hpp"
#include "types.hpp"
#include "board.hpp"
#include "attacks.hpp"

void generatePseudoLegalMoves(const Board& board, moveList& moves) {
    Bitboard knights = board.pieces[board.sideToMove][KNIGHT];
    Bitboard kings = board.pieces[board.sideToMove][KING];
    Bitboard bishops = board.pieces[board.sideToMove][BISHOP];
    Bitboard rooks = board.pieces[board.sideToMove][ROOK];
    Bitboard queens = board.pieces[board.sideToMove][QUEEN];
    Bitboard pawns = board.pieces[board.sideToMove][PAWN];
    Bitboard occupied = board.occupied();
    Bitboard us = board.colorPieces(board.sideToMove);
    Bitboard them = board.colorPieces(board.sideToMove == WHITE ? BLACK : WHITE);
    Bitboard all = us | them;
    Color stm = board.sideToMove;
    Color opp = stm == WHITE ? BLACK : WHITE;

    bool kc = stm == WHITE ? board.stateStack[board.ply].whiteKingsideCastling : board.stateStack[board.ply].blackKingsideCastling;
    bool qc = stm == WHITE ? board.stateStack[board.ply].whiteQueensideCastling : board.stateStack[board.ply].blackQueensideCastling;

    Square kingSq = stm == WHITE ? e1 : e8;

    Square ks1 = stm == WHITE ? f1 : f8;
    Square ks2 = stm == WHITE ? g1 : g8;

    Square qs1 = stm == WHITE ? d1 : d8;
    Square qs2 = stm == WHITE ? c1 : c8;
    Square qs3 = stm == WHITE ? b1 : b8;

    if (kc && !isSet(all, ks1) && !isSet(all, ks2) && !board.isSquareAttacked(ks1, opp) && !board.isSquareAttacked(ks2, opp) && !board.inCheck(stm)) {
        moves.push({kingSq, ks2, KING_CASTLE});
    }
    if (qc && !isSet(all, qs1) && !isSet(all, qs2) && !isSet(all, qs3) && !board.isSquareAttacked(qs1, opp) && !board.isSquareAttacked(qs2, opp) && !board.inCheck(stm)) {
        moves.push({kingSq, qs2, QUEEN_CASTLE});
    }

    while (knights) {
        int sq = popSq(knights);

        Bitboard attacks = attacks::KNIGHT_ATTACKS[sq];
        Bitboard knightMoves = attacks & ~us;
        while (knightMoves) {
            int to = popSq(knightMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (kings) {
        int sq = popSq(kings);

        Bitboard attacks = attacks::KING_ATTACKS[sq];
        Bitboard kingMoves = attacks & ~us;
        while (kingMoves) {
            int to = popSq(kingMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (bishops) {
        int sq = popSq(bishops);

        Bitboard attacks = bishopAttacks(sq, occupied);
        Bitboard bishopMoves = attacks & ~us;
        while (bishopMoves) {
            int to = popSq(bishopMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (rooks) {
        int sq = popSq(rooks);

        Bitboard attacks = rookAttacks(sq, occupied);
        Bitboard rookMoves = attacks & ~us;
        while (rookMoves) {
            int to = popSq(rookMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (queens) {
        int sq = popSq(queens);

        Bitboard attacks = bishopAttacks(sq, occupied) | rookAttacks(sq, occupied);
        Bitboard queenMoves = attacks & ~us;
        while (queenMoves) {
            int to = popSq(queenMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (pawns) {
        int from = popSq(pawns);
        int rank = from / 8;

        bool promotion = (board.sideToMove == WHITE && rank == 6) || (board.sideToMove == BLACK && rank == 1);
        bool doublePush = (board.sideToMove == WHITE && rank == 1) || (board.sideToMove == BLACK && rank == 6);

        Bitboard captures = attacks::PAWN_ATTACKS[board.sideToMove][from] & them;

        Square ep = board.stateStack[board.ply].enPassantSquare;

        if (ep != NO_SQUARE && isSet(attacks::PAWN_ATTACKS[board.sideToMove][from], ep)) {
            moves.push({static_cast<Square>(from), ep, EN_PASSANT});
        }

        while (captures) {
            int to = popSq(captures);
            if (promotion) {
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_KNIGHT});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_BISHOP});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_ROOK});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_QUEEN});
            } else {
                moves.push({static_cast<Square>(from), static_cast<Square>(to)});
            }
        }

        int to = board.sideToMove == WHITE ? from + 8 : from - 8;

        if (to >= 0 && to < 64 && !isSet(occupied, to)) {
            if (promotion) {
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_KNIGHT});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_BISHOP});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_ROOK});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_QUEEN});
            } else {
                moves.push({static_cast<Square>(from), static_cast<Square>(to)});
            }
        }

        if (doublePush) {
            int toDouble = board.sideToMove == WHITE ? from + 16 : from - 16;
            if (toDouble >= 0 && toDouble < 64 && !isSet(occupied, toDouble) && !isSet(occupied, to)) {
                moves.push({static_cast<Square>(from), static_cast<Square>(toDouble), DOUBLE_PAWN_PUSH});
            }
        }
    }
}

void generatePseudoLegalCaptures(const Board& board, moveList& moves) {
    Bitboard knights = board.pieces[board.sideToMove][KNIGHT];
    Bitboard kings = board.pieces[board.sideToMove][KING];
    Bitboard bishops = board.pieces[board.sideToMove][BISHOP];
    Bitboard rooks = board.pieces[board.sideToMove][ROOK];
    Bitboard queens = board.pieces[board.sideToMove][QUEEN];
    Bitboard pawns = board.pieces[board.sideToMove][PAWN];
    Bitboard occupied = board.occupied();
    Bitboard them = board.colorPieces(board.sideToMove == WHITE ? BLACK : WHITE);

    while (knights) {
        int sq = popSq(knights);

        Bitboard attacks = attacks::KNIGHT_ATTACKS[sq];
        Bitboard knightMoves = attacks & them;
        while (knightMoves) {
            int to = popSq(knightMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (kings) {
        int sq = popSq(kings);

        Bitboard attacks = attacks::KING_ATTACKS[sq];
        Bitboard kingMoves = attacks & them;
        while (kingMoves) {
            int to = popSq(kingMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (bishops) {
        int sq = popSq(bishops);

        Bitboard attacks = bishopAttacks(sq, occupied);
        Bitboard bishopMoves = attacks & them;
        while (bishopMoves) {
            int to = popSq(bishopMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (rooks) {
        int sq = popSq(rooks);

        Bitboard attacks = rookAttacks(sq, occupied);
        Bitboard rookMoves = attacks & them;
        while (rookMoves) {
            int to = popSq(rookMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (queens) {
        int sq = popSq(queens);

        Bitboard attacks = bishopAttacks(sq, occupied) | rookAttacks(sq, occupied);
        Bitboard queenMoves = attacks & them;
        while (queenMoves) {
            int to = popSq(queenMoves);
            moves.push({static_cast<Square>(sq), static_cast<Square>(to)});
        }
    }

    while (pawns) {
        int from = popSq(pawns);
        int rank = from / 8;

        bool promotion = (board.sideToMove == WHITE && rank == 6) || (board.sideToMove == BLACK && rank == 1);

        Bitboard captures = attacks::PAWN_ATTACKS[board.sideToMove][from] & them;

        Square ep = board.stateStack[board.ply].enPassantSquare;

        if (ep != NO_SQUARE && isSet(attacks::PAWN_ATTACKS[board.sideToMove][from], ep)) {
            moves.push({static_cast<Square>(from), ep, EN_PASSANT});
        }

        while (captures) {
            int to = popSq(captures);
            if (promotion) {
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_KNIGHT});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_BISHOP});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_ROOK});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_QUEEN});
            }
        }

        int to = board.sideToMove == WHITE ? from + 8 : from - 8;

        if (to >= 0 && to < 64 && !isSet(occupied, to)) {
            if (promotion) {
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_KNIGHT});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_BISHOP});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_ROOK});
                moves.push({static_cast<Square>(from), static_cast<Square>(to), PROMOTION_QUEEN});
            }
        }
    }
}