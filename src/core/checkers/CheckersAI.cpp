#include "CheckersAI.h"
#include <chrono>
#include <algorithm>
#include <limits>

namespace Strategix::Checkers {

CheckersAI::CheckersAI(AIDifficulty diff)
    : m_difficulty(diff)
    , m_rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()))
{
}

Move CheckersAI::chooseMove(const CheckersBoard& board, PieceColor color) {
    auto moves = board.getLegalMoves(color);
    if (moves.empty()) return Move{};
    if (moves.size() == 1) return moves[0];

    if (m_difficulty == AIDifficulty::Easy) {
        std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
        return moves[dist(m_rng)];
    }

    int maxDepth = (m_difficulty == AIDifficulty::Hard) ? 4 : 2;

    int bestScore = std::numeric_limits<int>::min();
    Move bestMove = moves[0];

    for (const auto& m : moves) {
        CheckersBoard copy = board;
        copy.applyMove(m);

        int score = minimax(copy, maxDepth - 1, -100000, 100000, false, color);
        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
    }

    return bestMove;
}

int CheckersAI::evaluate(const CheckersBoard& board, PieceColor myColor) const {
    int score = 0;

    for (int r = 0; r < CheckersBoard::SIZE; ++r) {
        for (int c = 0; c < CheckersBoard::SIZE; ++c) {
            Piece p = board.pieceAt(r, c);
            if (p.isEmpty()) continue;

            int pieceVal = p.isKing() ? 300 : 100;

            // Advancement bonus for men
            if (p.isMan()) {
                int advancement = p.isWhite() ? (CheckersBoard::SIZE - 1 - r) : r;
                pieceVal += advancement * 5;
            }

            // Center control bonus
            if ((r == 3 || r == 4) && (c >= 2 && c <= 5)) {
                pieceVal += 10;
            }

            if (p.color == myColor) {
                score += pieceVal;
            } else {
                score -= pieceVal;
            }
        }
    }

    return score;
}

int CheckersAI::minimax(CheckersBoard board, int depth, int alpha, int beta, bool maximizing, PieceColor aiColor) {
    PieceColor oppColor = (aiColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    PieceColor turnColor = maximizing ? aiColor : oppColor;

    auto moves = board.getLegalMoves(turnColor);

    if (moves.empty()) {
        // Player has no moves left -> loses!
        return maximizing ? -10000 : 10000;
    }

    if (depth == 0) {
        return evaluate(board, aiColor);
    }

    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& m : moves) {
            CheckersBoard copy = board;
            copy.applyMove(m);
            int eval = minimax(copy, depth - 1, alpha, beta, false, aiColor);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; // Beta cutoff
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& m : moves) {
            CheckersBoard copy = board;
            copy.applyMove(m);
            int eval = minimax(copy, depth - 1, alpha, beta, true, aiColor);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break; // Alpha cutoff
        }
        return minEval;
    }
}

} // namespace Strategix::Checkers
