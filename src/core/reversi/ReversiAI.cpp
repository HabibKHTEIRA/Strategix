#include "ReversiAI.h"
#include <algorithm>
#include <random>
#include <climits>

namespace Strategix::Reversi {

// Positional importance matrix for 8x8 Othello
static const int POS_WEIGHTS[8][8] = {
    { 120, -20,  20,   5,   5,  20, -20, 120 },
    { -20, -40,  -5,  -5,  -5,  -5, -40, -20 },
    {  20,  -5,  15,   3,   3,  15,  -5,  20 },
    {   5,  -5,   3,   3,   3,   3,  -5,   5 },
    {   5,  -5,   3,   3,   3,   3,  -5,   5 },
    {  20,  -5,  15,   3,   3,  15,  -5,  20 },
    { -20, -40,  -5,  -5,  -5,  -5, -40, -20 },
    { 120, -20,  20,   5,   5,  20, -20, 120 }
};

ReversiAI::ReversiAI(AIDifficulty diff)
    : m_difficulty(diff)
{
}

std::optional<Move> ReversiAI::chooseMove(const ReversiBoard& board, Piece aiColor) {
    auto legal = board.getLegalMoves(aiColor);
    if (legal.empty()) return std::nullopt;

    if (m_difficulty == AIDifficulty::Easy) {
        static std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
        return legal[dist(rng)];
    }

    int maxDepth = (m_difficulty == AIDifficulty::Medium) ? 2 : 4;

    Move bestMove = legal[0];
    int bestVal = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;

    for (const auto& mv : legal) {
        ReversiBoard nextBoard = board;
        nextBoard.applyMove(mv.row, mv.col, aiColor);

        int score = minimax(nextBoard, maxDepth - 1, alpha, beta, false, aiColor);
        if (score > bestVal) {
            bestVal = score;
            bestMove = mv;
        }
        alpha = std::max(alpha, bestVal);
    }

    return bestMove;
}

int ReversiAI::minimax(ReversiBoard& board, int depth, int alpha, int beta, bool maximizing, Piece aiColor) {
    Piece oppColor = opponent(aiColor);

    if (board.isGameOver()) {
        int aiCount = (aiColor == Piece::Black) ? board.blackCount() : board.whiteCount();
        int oppCount = (oppColor == Piece::Black) ? board.blackCount() : board.whiteCount();
        if (aiCount > oppCount) return 10000 + (aiCount - oppCount);
        if (aiCount < oppCount) return -10000 - (oppCount - aiCount);
        return 0;
    }

    if (depth == 0) {
        return evaluateBoard(board, aiColor);
    }

    Piece current = maximizing ? aiColor : oppColor;
    auto legal = board.getLegalMoves(current);

    if (legal.empty()) {
        // Player must pass
        return minimax(board, depth - 1, alpha, beta, !maximizing, aiColor);
    }

    if (maximizing) {
        int maxEval = INT_MIN;
        for (const auto& mv : legal) {
            ReversiBoard testBoard = board;
            testBoard.applyMove(mv.row, mv.col, aiColor);
            int eval = minimax(testBoard, depth - 1, alpha, beta, false, aiColor);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (const auto& mv : legal) {
            ReversiBoard testBoard = board;
            testBoard.applyMove(mv.row, mv.col, oppColor);
            int eval = minimax(testBoard, depth - 1, alpha, beta, true, aiColor);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

int ReversiAI::evaluateBoard(const ReversiBoard& board, Piece aiColor) const {
    Piece oppColor = opponent(aiColor);
    int posScore = 0;

    for (int r = 0; r < ReversiBoard::SIZE; ++r) {
        for (int c = 0; c < ReversiBoard::SIZE; ++c) {
            Piece p = board.pieceAt(r, c);
            if (p == aiColor) posScore += POS_WEIGHTS[r][c];
            else if (p == oppColor) posScore -= POS_WEIGHTS[r][c];
        }
    }

    // Mobility difference
    int aiMoves = static_cast<int>(board.getLegalMoves(aiColor).size());
    int oppMoves = static_cast<int>(board.getLegalMoves(oppColor).size());
    int mobilityScore = 0;
    if (aiMoves + oppMoves > 0) {
        mobilityScore = 100 * (aiMoves - oppMoves) / (aiMoves + oppMoves);
    }

    // Piece count
    int aiCount = (aiColor == Piece::Black) ? board.blackCount() : board.whiteCount();
    int oppCount = (oppColor == Piece::Black) ? board.blackCount() : board.whiteCount();
    int pieceScore = aiCount - oppCount;

    return posScore + 2 * mobilityScore + pieceScore;
}

} // namespace Strategix::Reversi
