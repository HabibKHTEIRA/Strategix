#include "MorrisGame.h"
#include <algorithm>

namespace Strategix::Morris {

MorrisGame::MorrisGame() {
    startNewGame();
}

void MorrisGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_board.reset();
    m_activePlayer = Piece::White;
    m_whiteUnplaced = 9;
    m_blackUnplaced = 9;
    m_awaitingCapture = false;
    m_status = GameStatus::InProgress;
    m_mode = mode;
    m_difficulty = diff;
    m_lastMove = std::nullopt;
    m_history.clear();
}

void MorrisGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.activePlayer = m_activePlayer;
    snap.whiteUnplaced = m_whiteUnplaced;
    snap.blackUnplaced = m_blackUnplaced;
    snap.awaitingCapture = m_awaitingCapture;
    snap.status = m_status;
    snap.lastMove = m_lastMove;
    m_history.push_back(snap);
}

GamePhase MorrisGame::currentPhase(Piece player) const {
    int unplaced = (player == Piece::White) ? m_whiteUnplaced : m_blackUnplaced;
    if (unplaced > 0) return GamePhase::Placing;
    if (m_board.pieceCount(player) == 3) return GamePhase::Flying;
    return GamePhase::Moving;
}

bool MorrisGame::placePiece(int pt) {
    if (m_status != GameStatus::InProgress || m_awaitingCapture) return false;
    if (currentPhase(m_activePlayer) != GamePhase::Placing) return false;
    if (pt < 0 || pt >= MorrisBoard::NUM_POINTS || m_board.pieceAt(pt) != Piece::Empty) return false;

    recordSnapshot();

    bool mill = m_board.createsMill(-1, pt, m_activePlayer);
    m_board.setPiece(pt, m_activePlayer);

    if (m_activePlayer == Piece::White) m_whiteUnplaced--;
    else m_blackUnplaced--;

    m_lastMove = Move{-1, pt};

    if (mill) {
        m_awaitingCapture = true;
    } else {
        endTurn();
    }
    return true;
}

bool MorrisGame::movePiece(int from, int to) {
    if (m_status != GameStatus::InProgress || m_awaitingCapture) return false;
    if (currentPhase(m_activePlayer) == GamePhase::Placing) return false;
    if (from < 0 || from >= MorrisBoard::NUM_POINTS || m_board.pieceAt(from) != m_activePlayer) return false;
    if (to < 0 || to >= MorrisBoard::NUM_POINTS || m_board.pieceAt(to) != Piece::Empty) return false;

    bool isFlying = (currentPhase(m_activePlayer) == GamePhase::Flying);
    auto legalMoves = m_board.getLegalMovesFrom(from, isFlying);
    if (std::find(legalMoves.begin(), legalMoves.end(), to) == legalMoves.end()) {
        return false;
    }

    recordSnapshot();

    bool mill = m_board.createsMill(from, to, m_activePlayer);
    m_board.setPiece(from, Piece::Empty);
    m_board.setPiece(to, m_activePlayer);

    m_lastMove = Move{from, to};

    if (mill) {
        m_awaitingCapture = true;
    } else {
        endTurn();
    }
    return true;
}

bool MorrisGame::removeOpponentPiece(int pt) {
    if (m_status != GameStatus::InProgress || !m_awaitingCapture) return false;

    Piece opp = opponent(m_activePlayer);
    if (m_board.pieceAt(pt) != opp) return false;

    auto capturable = m_board.getCapturablePieces(opp);
    if (std::find(capturable.begin(), capturable.end(), pt) == capturable.end()) {
        return false;
    }

    m_board.setPiece(pt, Piece::Empty);
    m_awaitingCapture = false;

    checkWinCondition();
    if (m_status == GameStatus::InProgress) {
        endTurn();
    }
    return true;
}

void MorrisGame::endTurn() {
    m_activePlayer = opponent(m_activePlayer);
    checkWinCondition();
}

void MorrisGame::checkWinCondition() {
    Piece opp = opponent(m_activePlayer);

    // If unplaced pieces remain, game cannot end by piece count yet
    int oppUnplaced = (opp == Piece::White) ? m_whiteUnplaced : m_blackUnplaced;
    if (oppUnplaced == 0 && m_board.pieceCount(opp) < 3) {
        m_status = (m_activePlayer == Piece::White) ? GameStatus::WhiteWon : GameStatus::BlackWon;
        return;
    }

    // Check if active player has no moves (in Moving/Flying phase)
    int currUnplaced = (m_activePlayer == Piece::White) ? m_whiteUnplaced : m_blackUnplaced;
    if (currUnplaced == 0) {
        bool isFlying = (m_board.pieceCount(m_activePlayer) == 3);
        if (m_board.getLegalMoves(m_activePlayer, isFlying).empty()) {
            m_status = (m_activePlayer == Piece::White) ? GameStatus::BlackWon : GameStatus::WhiteWon;
            return;
        }
    }
}

void MorrisGame::undo() {
    if (m_history.empty()) return;

    if (m_mode == GameMode::PlayerVsAI && m_history.size() >= 2) {
        m_history.pop_back();
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_whiteUnplaced = snap.whiteUnplaced;
        m_blackUnplaced = snap.blackUnplaced;
        m_awaitingCapture = snap.awaitingCapture;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
    } else {
        auto snap = m_history.back();
        m_history.pop_back();

        m_board = snap.board;
        m_activePlayer = snap.activePlayer;
        m_whiteUnplaced = snap.whiteUnplaced;
        m_blackUnplaced = snap.blackUnplaced;
        m_awaitingCapture = snap.awaitingCapture;
        m_status = snap.status;
        m_lastMove = snap.lastMove;
    }
}

} // namespace Strategix::Morris
