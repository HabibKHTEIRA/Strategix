#ifndef STRATEGIX_MORRISGAME_H
#define STRATEGIX_MORRISGAME_H

#include "MorrisBoard.h"
#include <vector>
#include <optional>

namespace Strategix::Morris {

enum class GamePhase {
    Placing,
    Moving,
    Flying
};

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

struct GameSnapshot {
    MorrisBoard board;
    Piece activePlayer;
    int whiteUnplaced;
    int blackUnplaced;
    bool awaitingCapture;
    GameStatus status;
    std::optional<Move> lastMove;
};

class MorrisGame {
public:
    MorrisGame();

    void startNewGame(GameMode mode = GameMode::TwoPlayers, AIDifficulty diff = AIDifficulty::Medium);

    const MorrisBoard& board() const { return m_board; }
    Piece activePlayer() const { return m_activePlayer; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }

    int whiteUnplaced() const { return m_whiteUnplaced; }
    int blackUnplaced() const { return m_blackUnplaced; }
    bool isAwaitingCapture() const { return m_awaitingCapture; }
    std::optional<Move> lastMove() const { return m_lastMove; }

    GamePhase currentPhase(Piece player) const;

    bool isAITurn() const {
        return m_mode == GameMode::PlayerVsAI && m_activePlayer == Piece::Black && m_status == GameStatus::InProgress;
    }

    // Phase 1: Place piece
    bool placePiece(int pt);

    // Phase 2/3: Move piece
    bool movePiece(int from, int to);

    // Capture opponent piece when mill formed
    bool removeOpponentPiece(int pt);

    std::vector<int> capturablePieces() const {
        return m_board.getCapturablePieces(opponent(m_activePlayer));
    }

    bool canUndo() const { return !m_history.empty(); }
    void undo();

private:
    void recordSnapshot();
    void endTurn();
    void checkWinCondition();

    MorrisBoard m_board;
    Piece m_activePlayer = Piece::White;
    int m_whiteUnplaced = 9;
    int m_blackUnplaced = 9;
    bool m_awaitingCapture = false;

    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::TwoPlayers;
    AIDifficulty m_difficulty = AIDifficulty::Medium;

    std::optional<Move> m_lastMove;
    std::vector<GameSnapshot> m_history;
};

} // namespace Strategix::Morris

#endif // STRATEGIX_MORRISGAME_H
