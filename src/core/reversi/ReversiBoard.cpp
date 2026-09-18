#include "ReversiBoard.h"

namespace Strategix::Reversi {

ReversiBoard::ReversiBoard() {
    reset();
}

void ReversiBoard::reset() {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            m_grid[r][c] = Piece::Empty;
        }
    }
    // Standard Othello starting configuration
    m_grid[3][3] = Piece::White;
    m_grid[3][4] = Piece::Black;
    m_grid[4][3] = Piece::Black;
    m_grid[4][4] = Piece::White;
}

Piece ReversiBoard::pieceAt(int r, int c) const {
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return Piece::Empty;
    return m_grid[r][c];
}

void ReversiBoard::setPiece(int r, int c, Piece p) {
    if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
        m_grid[r][c] = p;
    }
}

static const int DIRS[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1},          { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
};

std::vector<std::pair<int, int>> ReversiBoard::getFlippablePieces(int r, int c, Piece player) const {
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE || m_grid[r][c] != Piece::Empty) {
        return {};
    }

    Piece opp = opponent(player);
    std::vector<std::pair<int, int>> toFlip;

    for (const auto& d : DIRS) {
        int dr = d[0];
        int dc = d[1];
        int cr = r + dr;
        int cc = c + dc;
        std::vector<std::pair<int, int>> line;

        while (cr >= 0 && cr < SIZE && cc >= 0 && cc < SIZE && m_grid[cr][cc] == opp) {
            line.push_back({cr, cc});
            cr += dr;
            cc += dc;
        }

        if (!line.empty() && cr >= 0 && cr < SIZE && cc >= 0 && cc < SIZE && m_grid[cr][cc] == player) {
            toFlip.insert(toFlip.end(), line.begin(), line.end());
        }
    }

    return toFlip;
}

bool ReversiBoard::isValidMove(int r, int c, Piece player) const {
    return !getFlippablePieces(r, c, player).empty();
}

std::vector<Move> ReversiBoard::getLegalMoves(Piece player) const {
    std::vector<Move> moves;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (isValidMove(r, c, player)) {
                moves.push_back({r, c});
            }
        }
    }
    return moves;
}

int ReversiBoard::applyMove(int r, int c, Piece player) {
    auto flippable = getFlippablePieces(r, c, player);
    if (flippable.empty()) return 0;

    m_grid[r][c] = player;
    for (const auto& p : flippable) {
        m_grid[p.first][p.second] = player;
    }

    return static_cast<int>(flippable.size());
}

int ReversiBoard::blackCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c] == Piece::Black) count++;
        }
    }
    return count;
}

int ReversiBoard::whiteCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c] == Piece::White) count++;
        }
    }
    return count;
}

int ReversiBoard::emptyCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c] == Piece::Empty) count++;
        }
    }
    return count;
}

bool ReversiBoard::isGameOver() const {
    return getLegalMoves(Piece::Black).empty() && getLegalMoves(Piece::White).empty();
}

} // namespace Strategix::Reversi
