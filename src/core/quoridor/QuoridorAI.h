#ifndef STRATEGIX_QUORIDORAI_H
#define STRATEGIX_QUORIDORAI_H

#include "QuoridorBoard.h"
#include "QuoridorGame.h"

namespace Strategix::Quoridor {

struct AIAction {
    enum class Type {
        MovePawn,
        PlaceWall
    } type;

    Position targetPos;
    Wall wall;
};

class QuoridorAI {
public:
    explicit QuoridorAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    AIAction chooseAction(const QuoridorBoard& board, PlayerId aiPlayer, int aiWallsLeft);

private:
    int evaluate(const QuoridorBoard& board, PlayerId aiPlayer) const;
    std::vector<Wall> getCandidateWalls(const QuoridorBoard& board, PlayerId aiPlayer) const;

    AIDifficulty m_difficulty;
};

} // namespace Strategix::Quoridor

#endif // STRATEGIX_QUORIDORAI_H
