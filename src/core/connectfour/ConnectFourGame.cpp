#include "ConnectFourGame.h"

namespace Strategix::ConnectFour {

ConnectFourGame::ConnectFourGame() {
    startNewGame();
}

void ConnectFourGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_board.clear();
    m_activePlayer = Player::Red;
    m_status = GameStatus::InProgress;
    m_mode = mode;
    m_difficulty = diff;
    m_lastMove = {-1, -1};
    m_winningLine.clear();
    m_history.clear();
}

void ConnectFourGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.activePlayer = m_activePlayer;
    snap.status = m_status;
    snap.lastMove = m_lastMove;
    m_history.push_back(snap);
}

bool ConnectFourGame::dropPiece(int col) {
    if (m_status != GameStatus::InProgress) return false;
    if (col < 0 || col >= ConnectFourBoard::COLS) return false;

    recordSnapshot();

    int row = m_board.dropPiece(col, m_activePlayer);
    if (row == -1) {
        m_history.pop_back(); // Invalid move, discard snapshot
        return false;
    }

    m_lastMove = {row, col};

    auto win = m_board.getWinningLine(m_activePlayer);
    if (!win.empty()) {
        m_winningLine = win;
        m_status = (m_activePlayer == Player::Red) ? GameStatus::RedWon : GameStatus::YellowWon;
        return true;
    }

    if (m_board.isFull()) {
        m_status = GameStatus::Draw;
        return true;
    }

    m_activePlayer = opponent(m_activePlayer);
    return true;
}

void ConnectFourGame::undo() {
    if (m_history.empty()) return;

    if (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) {
        // Undo AI move and human move
        m_history.pop_back();
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
        m_winningLine.clear();
    } else {
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
        m_winningLine.clear();
    }
}

} // namespace Strategix::ConnectFour
