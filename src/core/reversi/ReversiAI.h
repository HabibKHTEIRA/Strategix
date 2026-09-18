#ifndef STRATEGIX_REVERSIAI_H
#define STRATEGIX_REVERSIAI_H

#include "ReversiBoard.h"
#include "ReversiGame.h"
#include <optional>

namespace Strategix::Reversi {

class ReversiAI {
public:
    explicit ReversiAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    std::optional<Move> chooseMove(const ReversiBoard& board, Piece aiColor);

private:
    int minimax(ReversiBoard& board, int depth, int alpha, int beta, bool maximizing, Piece aiColor);
    int evaluateBoard(const ReversiBoard& board, Piece aiColor) const;

    AIDifficulty m_difficulty;
};

} // namespace Strategix::Reversi

#endif // STRATEGIX_REVERSIAI_H
