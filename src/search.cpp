#include "search.hpp"
#include "evaluate.hpp"

int inf = 1'000'000'000;
int mateScore = 100'000;

std::array<TTEntry, 1048576> tt{};

std::array<Move, 256> killer1{};
std::array<Move, 256> killer2{};

std::array<std::array<int, 64>, 64> history{};
std::array<std::array<int, 64>, 64> butterfly{};

std::array<std::array<Move, 256>, 256> pvTable{};
std::array<int, 256> pvLength{};

void clearTT() {
    for (int i = 0; i < 1048576; i++) {
        tt[i].key = 0;
        tt[i].bestMove = Move::NO_MOVE;
    }
}

void clearKillers() {
    for (int i = 0; i < 256; i++) {
        killer1[i] = Move::NO_MOVE;
        killer2[i] = Move::NO_MOVE;
    }
}

void clearHistory() {
    for (int i = 0; i < 64; i++) {
        for (int x = 0; x < 64; x++) {
            history[i][x] = 0;
            butterfly[i][x] = 0;
        }
    }
}

int scoreToTT(int score, int ply) {
    if (score > 99000 && score <= 100000)
        return score + ply;

    if (score < -99000 && score >= -100000)
        return score - ply;

    return score;
}

int scoreFromTT(int score, int ply) {
    if (score > 99000 && score <= 100000)
        return score - ply;

    if (score < -99000 && score >= -100000)
        return score + ply;

    return score;
}

void storeTT(const Move& move, std::uint64_t key, int depth, int score, int flag, int ply) {
    int i = key & (1048576 - 1);
    if (move != Move::NO_MOVE) {
        tt[i].key = key;
        tt[i].depth = depth;
        tt[i].flag = flag;
        tt[i].bestMove = move;
        tt[i].score = scoreToTT(score, ply);
    }
}

int probeTT(int index, std::uint64_t key, int depth, int alpha, int beta, int ply) {
    if (tt[index].depth >= depth) {
        int score = scoreFromTT(tt[index].score, ply);

        if (tt[index].flag == ttExact) {
            return score;
        }

        if (tt[index].flag == ttAlpha && score <= alpha) {
            return score;
        }

        if (tt[index].flag == ttBeta && score >= beta) {
            return score;
        }

    }

    return -inf;
}

bool timeUp(const searchInfo& info) {
    return elapsedTime(info.start) >= info.timeLimit;
}

int pieceValue(PieceType piece) {
    if (piece == PieceType::PAWN) {
        return 100;
    } 
    if (piece == PieceType::KNIGHT) {
        return 300;
    }
    if (piece == PieceType::BISHOP) {
        return 320;
    }
    if (piece == PieceType::ROOK) {
        return 500;
    }
    if (piece == PieceType::QUEEN) {
        return 900;
    }
    if (piece == PieceType::KING) {
        return 20000;
    }
    
    return 0;
}

int mvvLva(const Board& board, const Move& move) {
    Square from = move.from();
    Square to = move.to();

    Piece attacker = board.at(from);
    Piece victim = board.at(to);

    int victimValue = move.typeOf() == Move::ENPASSANT ? 100 : pieceValue(victim.type());
    int attackerValue = pieceValue(attacker.type());

    return 900'000 + victimValue * 10 - attackerValue;
}

void scoreMoves(const Board& board, const Move& hint, Movelist& moves, int ply) {
    std::uint64_t ttKey = board.hash();
    int ttIndex = ttKey & (1048576 - 1);

    for (Move& move : moves) {
        int score = 0;

        if (tt[ttIndex].key == ttKey && tt[ttIndex].bestMove == move) {
            score = 2'000'000;
        } else if (move == hint && hint != Move::NO_MOVE) {
            score = 1'000'000;
        } else if (board.isCapture(move)) {
            score = mvvLva(board, move);
        } else if (move == killer1[ply]) {
            score = 890'000;
        } else if (move == killer2[ply]) {
            score = 880'000;
        } else {
            int from = move.from().index();
            int to = move.to().index();

            score = history[from][to] * 1000 / std::max(1, butterfly[from][to]);
        }

        move.setScore(score);
    }
}

int qsearch(Board& board, int ply, int alpha, int beta, searchInfo& info) {
    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    if ((info.nodes & 2047) == 0 && timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        return 0;
    }

    if (board.isInsufficientMaterial()) {
        return 0;
    }

    if (ply > 0) {
        if (board.isHalfMoveDraw()) {
            return board.getHalfMoveDrawType().first == GameResultReason::CHECKMATE ? -mateScore + ply : 0;
        }
        if (board.isRepetition(1)) {
            return 0;
        }
    }

    if (ply >= 245) {
        return evaluate(board);
    }

    Movelist moves;

    alpha = std::max(alpha, -mateScore + ply);
    beta = std::min(beta, mateScore - ply);

    if (alpha >= beta) {
        return alpha;
    }

    if (board.inCheck()) {
        movegen::legalmoves(moves, board);
        if (moves.empty()) {
            return -mateScore + ply;
        }
    } else {
        int standPat = evaluate(board);

        if (standPat >= beta) {
            return standPat;
        }

        if (standPat > alpha) {
            alpha = standPat;
        }

        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    }

    scoreMoves(board, Move::NO_MOVE, moves, ply);
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {return a.score() > b.score();});

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return 0;
        }

        const Move& move = moves[i];

        board.makeMove(move);

        int score = -qsearch(board, ply + 1, -beta, -alpha, info);

        board.unmakeMove(move);

        if (info.stop) {
            return 0;
        }

        if (score >= beta) {
            return score;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int negamax(Board& board, int depth, int alpha, int beta, int ply, searchInfo& info, const Move& hint, bool allowNullMove) {
    int bestScore = -inf;
    
    pvLength[ply] = ply;

    info.nodes++;
    info.selDepth = std::max(info.selDepth, ply);

    std::uint64_t ttKey = board.hash();
    int ttIndex = ttKey & (1048576 - 1);

    if ((info.nodes & 2047) == 0 && timeUp(info)) {
        info.stop = true;
    }

    if (info.stop) {
        return 0;
    }

    if (board.isInsufficientMaterial()) {
        return 0;
    }

    if (ply > 0) {
        if (board.isHalfMoveDraw()) {
            return board.getHalfMoveDrawType().first == GameResultReason::CHECKMATE ? -mateScore + ply : 0;
        }
        if (board.isRepetition(1)) {
            return 0;
        }
    }

    if (depth < 0) {
        depth = 0;
    }

    alpha = std::max(alpha, -mateScore + ply);
    beta = std::min(beta, mateScore - ply);

    if (alpha >= beta) {
        return alpha;
    }

    int originalAlpha = alpha;

    if (ttKey == tt[ttIndex].key) {
        int ttScore = probeTT(ttIndex, ttKey, depth, alpha, beta, ply);
        if (ttScore != -inf) {
            pvTable[ply][ply] = tt[ttIndex].bestMove;
            pvLength[ply] = ply + 1;

            return ttScore;
        }
    }

    if (depth == 0) {
        return qsearch(board, ply, alpha, beta, info);
    }

    if (ply >= 245) {
        return evaluate(board);
    }

    int extension = board.inCheck() ? 1 : 0;

    bool canNullMove = ply > 0 && extension == 0 && depth >= 4 && std::abs(beta) < 90000 && board.hasNonPawnMaterial(board.sideToMove()) && allowNullMove;

    if (canNullMove) {
        board.makeNullMove();

        int score = -negamax(board, depth - 1 - 2, -beta, -beta + 1, ply + 1, info, Move::NO_MOVE, false);

        board.unmakeNullMove();

        if (!info.stop && score >= beta) {
            return score;
        }
    }

    bool pvNode = beta - alpha > 1;

    if (!pvNode && extension == 0 && depth <= 4 && std::abs(beta) < 90000) {
        int eval = evaluate(board);
        if (eval - (depth * 100) >= beta) {
            return eval;
        }
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.empty()) {
        return board.inCheck() ? -mateScore + ply : 0;
    }

    scoreMoves(board, hint, moves, ply);
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {return a.score() > b.score();});

    for (int i = 0; i < moves.size(); i++) {
        if (info.stop) {
            return 0;
        }

        const Move& move = moves[i];

        bool isCapture = board.isCapture(move);

        int from = move.from().index();
        int to = move.to().index();

        bool lmrPossible = (!isCapture && extension == 0 && depth >= 4 && i >= 5);
        
        board.makeMove(move);

        int score;

        if (i == 0) {
            score = -negamax(board, depth - 1 + extension, -beta, -alpha, ply + 1, info, Move::NO_MOVE, true);
        } else if (lmrPossible) {
            score = -negamax(board, depth - 1 - 2, -alpha - 1, -alpha, ply + 1, info, Move::NO_MOVE, true);
            if (!info.stop && score > alpha) {
                score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, info, Move::NO_MOVE, true);
            }
        } else {
            score = -negamax(board, depth - 1 + extension, -alpha - 1, -alpha, ply + 1, info, Move::NO_MOVE, true);
            if (!info.stop && score > alpha && score < beta) {
                score = -negamax(board, depth - 1 + extension, -beta, -alpha, ply + 1, info, Move::NO_MOVE, true);
            }
        }

        board.unmakeMove(move);

        if (info.stop) {
            return 0;
        }

        if (!isCapture) {
            butterfly[from][to] += 1;
            butterfly[from][to] = std::min(butterfly[from][to], 250'000);
        }

        if (score > bestScore) {
            bestScore = score;

            pvTable[ply][ply] = move;

            for (int i = ply + 1; i < pvLength[ply + 1];  i++) {
                pvTable[ply][i] = pvTable[ply + 1][i];
            }

            pvLength[ply] = pvLength[ply + 1];
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            if (!isCapture) {
                if (killer1[ply] != move) {
                    killer2[ply] = killer1[ply];
                    killer1[ply] = move;
                }

                history[from][to] += depth * depth;
                history[from][to] = std::min(history[from][to], 250'000);
            }

            storeTT(move, ttKey, depth, bestScore, ttBeta, ply);
            return bestScore;
        }
    }

    if (alpha != originalAlpha) {
        storeTT(pvTable[ply][ply], ttKey, depth, bestScore, ttExact, ply);
    } else {
        storeTT(pvTable[ply][ply], ttKey, depth, bestScore, ttAlpha, ply);
    }

    return bestScore;
}