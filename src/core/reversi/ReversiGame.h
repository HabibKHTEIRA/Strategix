#ifndef STRATEGIX_REVERSIGAME_H
#define STRATEGIX_REVERSIGAME_H

#include "ReversiBoard.h"
#include <vector>
#include <optional>

namespace Strategix::Reversi {

enum class GameStatus {
    InProgress,
    BlackWon,
    WhiteWon,
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
    ReversiBoard board;
    Piece activePlayer;
    GameStatus status;
    std::optional<Move> lastMove;
};

class ReversiGame {
public:
    ReversiGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const ReversiBoard& board() const { return m_board; }
    Piece activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }
    std::optional<Move> lastMove() const { return m_lastMove; }

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == Piece::White && m_status == GameStatus::InProgress;
    }

    bool playMove(int r, int c);
    std::vector<Move> legalMoves() const { return m_board.getLegalMoves(m_activePlayer); }

    int blackScore() const { return m_board.blackCount(); }
    int whiteScore() const { return m_board.whiteCount(); }

    bool canUndo() const { return !m_history.empty(); }
    void undo();

private:
    void recordSnapshot();
    void advanceTurnOrFinish();

    ReversiBoard m_board;
    Piece m_activePlayer = Piece::Black;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    std::optional<Move> m_lastMove;
    std::vector<GameSnapshot> m_history;
};

} // namespace Strategix::Reversi

#endif // STRATEGIX_REVERSIGAME_H
