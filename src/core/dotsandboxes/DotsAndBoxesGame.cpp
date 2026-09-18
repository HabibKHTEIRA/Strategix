#include "DotsAndBoxesGame.h"

namespace Strategix::DotsAndBoxes {

DotsAndBoxesGame::DotsAndBoxesGame() {
    startNewGame();
}

void DotsAndBoxesGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_board.reset();
    m_activePlayer = Player::Player1;
    m_status = GameStatus::InProgress;
    m_mode = mode;
    m_difficulty = diff;
    m_lastEdge = std::nullopt;
    m_history.clear();
}

void DotsAndBoxesGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.activePlayer = m_activePlayer;
    snap.status = m_status;
    snap.lastEdge = m_lastEdge;
    m_history.push_back(snap);
}

bool DotsAndBoxesGame::playEdge(const Edge& edge) {
    if (m_status != GameStatus::InProgress) return false;
    if (m_board.isEdgeDrawn(edge)) return false;

    recordSnapshot();

    m_board.drawEdge(edge, m_activePlayer);
    m_lastEdge = edge;

    if (m_board.isFull()) {
        int s1 = m_board.score(Player::Player1);
        int s2 = m_board.score(Player::Player2);
        if (s1 > s2) m_status = GameStatus::Player1Won;
        else if (s2 > s1) m_status = GameStatus::Player2Won;
        else m_status = GameStatus::Draw;
        return true;
    }

    // Closing a box awards points, but turn always passes to the next player
    m_activePlayer = opponent(m_activePlayer);
    return true;
}

void DotsAndBoxesGame::undo() {
    if (m_history.empty()) return;

    if (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) {
        // Find last human player turn
        while (!m_history.empty()) {
            auto snap = m_history.back();
            m_history.pop_back();
            if (snap.activePlayer == Player::Player1) {
                m_board = snap.board;
                m_activePlayer = snap.activePlayer;
                m_status = snap.status;
                m_lastEdge = snap.lastEdge;
                break;
            }
        }
    } else {
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_status = snap.status;
        m_lastEdge = snap.lastEdge;
    }
}

} // namespace Strategix::DotsAndBoxes
