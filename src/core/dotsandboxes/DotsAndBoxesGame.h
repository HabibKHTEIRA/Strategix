#ifndef STRATEGIX_DOTSANDBOXESGAME_H
#define STRATEGIX_DOTSANDBOXESGAME_H

#include "DotsAndBoxesBoard.h"
#include <vector>
#include <optional>

namespace Strategix::DotsAndBoxes {

enum class GameStatus {
    InProgress,
    Player1Won,
    Player2Won,
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
    DotsAndBoxesBoard board;
    Player activePlayer;
    GameStatus status;
    std::optional<Edge> lastEdge;
};

class DotsAndBoxesGame {
public:
    DotsAndBoxesGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const DotsAndBoxesBoard& board() const { return m_board; }
    Player activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }
    std::optional<Edge> lastEdge() const { return m_lastEdge; }

    int player1Score() const { return m_board.score(Player::Player1); }
    int player2Score() const { return m_board.score(Player::Player2); }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == Player::Player2 && m_status == GameStatus::InProgress;
    }

    // Returns true if move succeeded
    bool playEdge(const Edge& edge);

    bool canUndo() const { return !m_history.empty(); }
    void undo();

private:
    void recordSnapshot();

    DotsAndBoxesBoard m_board;
    Player m_activePlayer = Player::Player1;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    std::optional<Edge> m_lastEdge;
    std::vector<GameSnapshot> m_history;
};

} // namespace Strategix::DotsAndBoxes

#endif // STRATEGIX_DOTSANDBOXESGAME_H
