#include "QuoridorGame.h"

namespace Strategix::Quoridor {

QuoridorGame::QuoridorGame() {
    startNewGame();
}

void QuoridorGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_board.reset();
    m_activePlayer = PlayerId::Player1;
    m_p1Walls = 10;
    m_p2Walls = 10;
    m_status = GameStatus::InProgress;
    m_mode = mode;
    m_difficulty = diff;
    m_history.clear();
}

void QuoridorGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.activePlayer = m_activePlayer;
    snap.p1Walls = m_p1Walls;
    snap.p2Walls = m_p2Walls;
    snap.status = m_status;
    m_history.push_back(snap);
}

bool QuoridorGame::movePawn(Position dest) {
    if (m_status != GameStatus::InProgress) return false;
    if (!m_board.isValidPawnMove(m_activePlayer, dest)) return false;

    recordSnapshot();

    m_board.movePawn(m_activePlayer, dest);

    if (m_board.hasWon(m_activePlayer)) {
        m_status = (m_activePlayer == PlayerId::Player1) ? GameStatus::Player1Won : GameStatus::Player2Won;
        return true;
    }

    m_activePlayer = opponent(m_activePlayer);
    return true;
}

bool QuoridorGame::placeWall(const Wall& wall) {
    if (m_status != GameStatus::InProgress) return false;
    if (activePlayerWalls() <= 0) return false;
    if (!m_board.canPlaceWall(wall)) return false;

    recordSnapshot();

    m_board.placeWall(wall);

    if (m_activePlayer == PlayerId::Player1) {
        m_p1Walls--;
    } else {
        m_p2Walls--;
    }

    m_activePlayer = opponent(m_activePlayer);
    return true;
}

void QuoridorGame::undo() {
    if (m_history.empty()) return;

    if (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) {
        m_history.pop_back();
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_p1Walls = snap.p1Walls;
        m_p2Walls = snap.p2Walls;
        m_status = snap.status;
    } else {
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_p1Walls = snap.p1Walls;
        m_p2Walls = snap.p2Walls;
        m_status = snap.status;
    }
}

} // namespace Strategix::Quoridor
