#ifndef STRATEGIX_CHECKERSGAME_H
#define STRATEGIX_CHECKERSGAME_H

#include "CheckersBoard.h"
#include <vector>

namespace Strategix::Checkers {

enum class GameStatus {
    InProgress,
    WhiteWon,
    BlackWon,
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

struct GameStateSnapshot {
    CheckersBoard board;
    PieceColor activePlayer;
    int whiteCaptures;
    int blackCaptures;
};

class CheckersGame {
public:
    CheckersGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const CheckersBoard& board() const { return m_board; }
    PieceColor activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int whiteCaptures() const { return m_whiteCaptures; }
    int blackCaptures() const { return m_blackCaptures; }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == PieceColor::Black && m_status == GameStatus::InProgress;
    }

    std::vector<Move> getLegalMoves() const {
        return m_board.getLegalMoves(m_activePlayer);
    }

    std::vector<Move> getLegalMovesFrom(Position from) const {
        return m_board.getLegalMovesFrom(from, m_activePlayer);
    }

    bool makeMove(const Move& move);

    bool canUndo() const;
    bool undo();

private:
    void checkGameEnd();

    CheckersBoard m_board;
    PieceColor m_activePlayer = PieceColor::White;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    int m_whiteCaptures = 0;
    int m_blackCaptures = 0;

    std::vector<GameStateSnapshot> m_history;
};

} // namespace Strategix::Checkers

#endif // STRATEGIX_CHECKERSGAME_H
