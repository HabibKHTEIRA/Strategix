#include "ReversiGame.h"

namespace Strategix::Reversi {

ReversiGame::ReversiGame() {
    startNewGame();
}

void ReversiGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_board.reset();
    m_activePlayer = Piece::Black;
    m_status = GameStatus::InProgress;
    m_mode = mode;
    m_difficulty = diff;
    m_lastMove = std::nullopt;
    m_history.clear();
}

void ReversiGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.activePlayer = m_activePlayer;
    snap.status = m_status;
    snap.lastMove = m_lastMove;
    m_history.push_back(snap);
}

bool ReversiGame::playMove(int r, int c) {
    if (m_status != GameStatus::InProgress) return false;
    if (!m_board.isValidMove(r, c, m_activePlayer)) return false;

    recordSnapshot();

    m_board.applyMove(r, c, m_activePlayer);
    m_lastMove = Move{r, c};

    advanceTurnOrFinish();
    return true;
}

void ReversiGame::advanceTurnOrFinish() {
    Piece opp = opponent(m_activePlayer);
    bool oppHasMoves = !m_board.getLegalMoves(opp).empty();
    bool currHasMoves = !m_board.getLegalMoves(m_activePlayer).empty();

    if (oppHasMoves) {
        m_activePlayer = opp;
    } else if (currHasMoves) {
        // Opponent has no moves, player keeps turn (pass)
    } else {
        // Neither player has moves -> game over!
        int bCount = m_board.blackCount();
        int wCount = m_board.whiteCount();
        if (bCount > wCount) {
            m_status = GameStatus::BlackWon;
        } else if (wCount > bCount) {
            m_status = GameStatus::WhiteWon;
        } else {
            m_status = GameStatus::Draw;
        }
    }
}

void ReversiGame::undo() {
    if (m_history.empty()) return;

    if (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) {
        m_history.pop_back();
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
    } else {
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
    }
}

} // namespace Strategix::Reversi
