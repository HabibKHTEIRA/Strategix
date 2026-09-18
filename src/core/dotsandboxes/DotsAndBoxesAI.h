#ifndef STRATEGIX_DOTSANDBOXESAI_H
#define STRATEGIX_DOTSANDBOXESAI_H

#include "DotsAndBoxesBoard.h"
#include "DotsAndBoxesGame.h"
#include <optional>

namespace Strategix::DotsAndBoxes {

class DotsAndBoxesAI {
public:
    explicit DotsAndBoxesAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    std::optional<Edge> chooseEdge(const DotsAndBoxesBoard& board, Player aiPlayer);

private:
    AIDifficulty m_difficulty;
};

} // namespace Strategix::DotsAndBoxes

#endif // STRATEGIX_DOTSANDBOXESAI_H
