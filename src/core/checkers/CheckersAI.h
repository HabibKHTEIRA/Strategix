#ifndef STRATEGIX_CHECKERSAI_H
#define STRATEGIX_CHECKERSAI_H

#include "CheckersGame.h"
#include <random>

namespace Strategix::Checkers {

class CheckersAI {
public:
    explicit CheckersAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    Move chooseMove(const CheckersBoard& board, PieceColor color);

private:
    int evaluate(const CheckersBoard& board, PieceColor myColor) const;
    int minimax(CheckersBoard board, int depth, int alpha, int beta, bool maximizing, PieceColor aiColor);

    AIDifficulty m_difficulty;
    std::mt19937 m_rng;
};

} // namespace Strategix::Checkers

#endif // STRATEGIX_CHECKERSAI_H
