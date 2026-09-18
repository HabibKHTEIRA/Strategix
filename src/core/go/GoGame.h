#ifndef STRATEGIX_GOGAME_H
#define STRATEGIX_GOGAME_H

#include "GoBoard.h"
#include <vector>
#include <optional>

namespace Strategix::Go {

enum class GameStatus {
    InProgress,
    Finished
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

struct GoStateSnapshot {
    GoBoard board;
    StoneColor activePlayer;
    int blackCaptures;
    int whiteCaptures;
    int consecutivePasses;
    std::optional<Point> koPoint;
    std::optional<Point> lastMovePoint;
};

class GoGame {
public:
    explicit GoGame(int boardSize = GoBoard::DEFAULT_SIZE);

    void startNewGame(GameMode mode = GameMode::TwoPlayers,
                      AIDifficulty diff = AIDifficulty::Medium,
                      int boardSize = GoBoard::DEFAULT_SIZE);

    const GoBoard& board() const { return m_board; }
    StoneColor activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int blackCaptures() const { return m_blackCaptures; }
    int whiteCaptures() const { return m_whiteCaptures; }
    std::optional<Point> lastMovePoint() const { return m_lastMovePoint; }
    std::optional<Point> koPoint() const { return m_koPoint; }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == StoneColor::White && m_status == GameStatus::InProgress;
    }

    bool isLegalMove(Point p) const {
        return m_board.isLegalMove(p, m_activePlayer, m_koPoint);
    }

    bool playMove(Point p);
    void passTurn();

    // Final scores calculation (Tromp-Taylor / territory + captures + komi)
    double blackScore() const;
    double whiteScore() const;
    StoneColor winner() const;

    bool canUndo() const;
    bool undo();

private:
    void finishGame();

    GoBoard m_board;
    StoneColor m_activePlayer = StoneColor::Black; // Black moves first in Go
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    int m_blackCaptures = 0;
    int m_whiteCaptures = 0;
    int m_consecutivePasses = 0;
    std::optional<Point> m_koPoint;
    std::optional<Point> m_lastMovePoint;

    double m_komi = 5.5; // Komi compensation for White

    std::vector<GoStateSnapshot> m_history;
};

} // namespace Strategix::Go

#endif // STRATEGIX_GOGAME_H
