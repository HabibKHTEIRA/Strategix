#include "CheckersGame.h"

namespace Strategix::Checkers {

CheckersGame::CheckersGame() {
    startNewGame();
}

void CheckersGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_mode = mode;
    m_difficulty = diff;
    m_board.reset();
    m_activePlayer = PieceColor::White;
    m_status = GameStatus::InProgress;
    m_whiteCaptures = 0;
    m_blackCaptures = 0;
    m_history.clear();
}

bool CheckersGame::makeMove(const Move& move) {
    if (m_status != GameStatus::InProgress) {
        return false;
    }

    // Save snapshot for undo
    m_history.push_back({m_board, m_activePlayer, m_whiteCaptures, m_blackCaptures});

    if (!move.captured.empty()) {
        if (m_activePlayer == PieceColor::White) {
            m_whiteCaptures += static_cast<int>(move.captured.size());
        } else {
            m_blackCaptures += static_cast<int>(move.captured.size());
        }
    }

    m_board.applyMove(move);

    // Switch turn
    m_activePlayer = (m_activePlayer == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    // Check game over
    checkGameEnd();

    return true;
}

void CheckersGame::checkGameEnd() {
    // 1. Check piece count
    int whitePieces = m_board.whiteCount();
    int blackPieces = m_board.blackCount();

    if (whitePieces == 0) {
        m_status = GameStatus::BlackWon;
        return;
    }
    if (blackPieces == 0) {
        m_status = GameStatus::WhiteWon;
        return;
    }

    // 2. Check legal moves
    auto legal = m_board.getLegalMoves(m_activePlayer);
    if (legal.empty()) {
        // Player has no moves left -> loses!
        m_status = (m_activePlayer == PieceColor::White) ? GameStatus::BlackWon : GameStatus::WhiteWon;
    }
}

bool CheckersGame::canUndo() const {
    return !m_history.empty() && m_status == GameStatus::InProgress;
}

bool CheckersGame::undo() {
    if (!canUndo()) return false;

    // If vs AI, undo 2 steps (AI move + player move)
    int steps = (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) ? 2 : 1;
    for (int s = 0; s < steps && !m_history.empty(); ++s) {
        auto snapshot = m_history.back();
        m_history.pop_back();

        m_board = snapshot.board;
        m_activePlayer = snapshot.activePlayer;
        m_whiteCaptures = snapshot.whiteCaptures;
        m_blackCaptures = snapshot.blackCaptures;
    }

    m_status = GameStatus::InProgress;
    return true;
}

} // namespace Strategix::Checkers
