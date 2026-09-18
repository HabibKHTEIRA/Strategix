#ifndef STRATEGIX_CONNECTFOURAI_H
#define STRATEGIX_CONNECTFOURAI_H

#include "ConnectFourBoard.h"
#include "ConnectFourGame.h"

namespace Strategix::ConnectFour {

class ConnectFourAI {
public:
    explicit ConnectFourAI(AIDifficulty diff = AIDifficulty::Medium);

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int chooseColumn(const ConnectFourBoard& board, Player aiPlayer);

private:
    int minimax(ConnectFourBoard& board, int depth, int alpha, int beta, bool maximizing, Player aiPlayer);
    int evaluateBoard(const ConnectFourBoard& board, Player aiPlayer) const;
    int evaluateWindow(const std::array<Player, 4>& window, Player aiPlayer) const;

    AIDifficulty m_difficulty;
};

} // namespace Strategix::ConnectFour

#endif // STRATEGIX_CONNECTFOURAI_H
