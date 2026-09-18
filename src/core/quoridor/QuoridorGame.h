#ifndef STRATEGIX_QUORIDORGAME_H
#define STRATEGIX_QUORIDORGAME_H

#include "QuoridorBoard.h"
#include <vector>

namespace Strategix::Quoridor {

enum class GameStatus {
    InProgress,
    Player1Won,
    Player2Won
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
    QuoridorBoard board;
    PlayerId activePlayer;
    int p1Walls;
    int p2Walls;
    GameStatus status;
};

class QuoridorGame {
public:
    QuoridorGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const QuoridorBoard& board() const { return m_board; }
    PlayerId activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int p1Walls() const { return m_p1Walls; }
    int p2Walls() const { return m_p2Walls; }
    int activePlayerWalls() const { return (m_activePlayer == PlayerId::Player1) ? m_p1Walls : m_p2Walls; }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == PlayerId::Player2 && m_status == GameStatus::InProgress;
    }

    bool movePawn(Position dest);
    bool placeWall(const Wall& wall);

    bool canUndo() const { return !m_history.empty(); }
    void undo();

private:
    void recordSnapshot();

    QuoridorBoard m_board;
    PlayerId m_activePlayer = PlayerId::Player1;
    int m_p1Walls = 10;
    int m_p2Walls = 10;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    std::vector<GameSnapshot> m_history;
};

} // namespace Strategix::Quoridor

#endif // STRATEGIX_QUORIDORGAME_H
