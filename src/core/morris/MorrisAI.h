#ifndef STRATEGIX_MORRISAI_H
#define STRATEGIX_MORRISAI_H

#include "MorrisBoard.h"
#include "MorrisGame.h"

namespace Strategix::Morris {

class MorrisAI {
public:
    explicit MorrisAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int choosePlacement(const MorrisBoard& board, Piece aiColor);
    Move chooseMove(const MorrisBoard& board, Piece aiColor, bool isFlying);
    int chooseCapture(const MorrisBoard& board, Piece aiColor);

private:
    AIDifficulty m_difficulty;
};

} // namespace Strategix::Morris

#endif // STRATEGIX_MORRISAI_H
