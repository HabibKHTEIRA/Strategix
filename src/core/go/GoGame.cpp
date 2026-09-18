#include "GoGame.h"

namespace Strategix::Go {

GoGame::GoGame(int boardSize)
    : m_board(boardSize)
{
    startNewGame();
}

void GoGame::startNewGame(GameMode mode, AIDifficulty diff, int boardSize) {
    m_mode = mode;
    m_difficulty = diff;
    m_board = GoBoard(boardSize);
    m_activePlayer = StoneColor::Black;
    m_status = GameStatus::InProgress;
    m_blackCaptures = 0;
    m_whiteCaptures = 0;
    m_consecutivePasses = 0;
    m_koPoint = std::nullopt;
    m_lastMovePoint = std::nullopt;
    m_history.clear();
}

bool GoGame::playMove(Point p) {
    if (m_status != GameStatus::InProgress || !isLegalMove(p)) {
        return false;
    }

    // Save snapshot
    m_history.push_back({m_board, m_activePlayer, m_blackCaptures, m_whiteCaptures, m_consecutivePasses, m_koPoint, m_lastMovePoint});

    // Play move and capture
    auto captured = m_board.playMove(p, m_activePlayer);

    if (m_activePlayer == StoneColor::Black) {
        m_blackCaptures += static_cast<int>(captured.size());
    } else {
        m_whiteCaptures += static_cast<int>(captured.size());
    }

    // Check Ko rule: if exactly 1 stone was captured, and placing stone has 1 liberty which is the captured stone
    if (captured.size() == 1) {
        Group grp = m_board.getGroup(p);
        if (grp.stones.size() == 1 && grp.liberties.size() == 1 && *grp.liberties.begin() == captured[0]) {
            m_koPoint = captured[0];
        } else {
            m_koPoint = std::nullopt;
        }
    } else {
        m_koPoint = std::nullopt;
    }

    m_lastMovePoint = p;
    m_consecutivePasses = 0;

    // Switch turn
    m_activePlayer = opponentColor(m_activePlayer);
    return true;
}

void GoGame::passTurn() {
    if (m_status != GameStatus::InProgress) return;

    m_history.push_back({m_board, m_activePlayer, m_blackCaptures, m_whiteCaptures, m_consecutivePasses, m_koPoint, m_lastMovePoint});

    m_koPoint = std::nullopt;
    m_lastMovePoint = std::nullopt;
    ++m_consecutivePasses;

    if (m_consecutivePasses >= 2) {
        finishGame();
    } else {
        m_activePlayer = opponentColor(m_activePlayer);
    }
}

void GoGame::finishGame() {
    m_status = GameStatus::Finished;
}

double GoGame::blackScore() const {
    auto [blackTerritory, whiteTerritory] = m_board.countTerritory();
    (void)whiteTerritory;
    return blackTerritory + m_blackCaptures;
}

double GoGame::whiteScore() const {
    auto [blackTerritory, whiteTerritory] = m_board.countTerritory();
    (void)blackTerritory;
    return whiteTerritory + m_whiteCaptures + m_komi;
}

StoneColor GoGame::winner() const {
    if (m_status != GameStatus::Finished) return StoneColor::None;
    return (blackScore() > whiteScore()) ? StoneColor::Black : StoneColor::White;
}

bool GoGame::canUndo() const {
    return !m_history.empty() && m_status == GameStatus::InProgress;
}

bool GoGame::undo() {
    if (!canUndo()) return false;

    int steps = (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) ? 2 : 1;
    for (int s = 0; s < steps && !m_history.empty(); ++s) {
        auto snapshot = m_history.back();
        m_history.pop_back();

        m_board = snapshot.board;
        m_activePlayer = snapshot.activePlayer;
        m_blackCaptures = snapshot.blackCaptures;
        m_whiteCaptures = snapshot.whiteCaptures;
        m_consecutivePasses = snapshot.consecutivePasses;
        m_koPoint = snapshot.koPoint;
        m_lastMovePoint = snapshot.lastMovePoint;
    }

    m_status = GameStatus::InProgress;
    return true;
}

} // namespace Strategix::Go
