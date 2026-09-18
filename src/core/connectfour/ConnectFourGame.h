#ifndef STRATEGIX_CONNECTFOURGAME_H
#define STRATEGIX_CONNECTFOURGAME_H

#include "ConnectFourBoard.h"
#include <vector>

namespace Strategix::ConnectFour {

enum class GameStatus {
    InProgress,
    RedWon,
    YellowWon,
    Draw
};

enum class GameMode {
    TwoPlayers,
    PlayerVsAI
};

enum class AIDifficulty {
    Easy,
    Medium,
    Hard
};

struct GameSnapshot {
    ConnectFourBoard board;
    Player activePlayer;
    GameStatus status;
    std::pair<int, int> lastMove; // (row, col)
};

class ConnectFourGame {
public:
    ConnectFourGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const ConnectFourBoard& board() const { return m_board; }
    Player activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }
    std::pair<int, int> lastMove() const { return m_lastMove; }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == Player::Yellow && m_status == GameStatus::InProgress;
    }

    bool dropPiece(int col);
    bool canUndo() const { return !m_history.empty(); }
    void undo();

    const std::vector<std::pair<int, int>>& winningLine() const { return m_winningLine; }

private:
    void recordSnapshot();

    ConnectFourBoard m_board;
    Player m_activePlayer = Player::Red;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    std::pair<int, int> m_lastMove = {-1, -1};
    std::vector<std::pair<int, int>> m_winningLine;
    std::vector<GameSnapshot> m_history;
};

} // namespace Strategix::ConnectFour

#endif // STRATEGIX_CONNECTFOURGAME_H
