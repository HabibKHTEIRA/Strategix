#ifndef STRATEGIX_GOAI_H
#define STRATEGIX_GOAI_H

#include "GoGame.h"
#include <random>
#include <optional>

namespace Strategix::Go {

class GoAI {
public:
    explicit GoAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    // Returns a Point to play, or std::nullopt to PASS
    std::optional<Point> chooseMove(const GoBoard& board, StoneColor myColor, const std::optional<Point>& koPoint);

private:
    AIDifficulty m_difficulty;
    std::mt19937 m_rng;
};

} // namespace Strategix::Go

#endif // STRATEGIX_GOAI_H
