#include "ConnectFourBoard.h"

namespace Strategix::ConnectFour {

ConnectFourBoard::ConnectFourBoard() {
    clear();
}

void ConnectFourBoard::clear() {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            m_grid[r][c] = Player::None;
        }
    }
    m_pieceCount = 0;
}

Player ConnectFourBoard::cell(int r, int c) const {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return Player::None;
    return m_grid[r][c];
}

bool ConnectFourBoard::isColumnFull(int c) const {
    if (c < 0 || c >= COLS) return true;
    return m_grid[ROWS - 1][c] != Player::None;
}

int ConnectFourBoard::lowestEmptyRow(int c) const {
    if (c < 0 || c >= COLS) return -1;
    for (int r = 0; r < ROWS; ++r) {
        if (m_grid[r][c] == Player::None) return r;
    }
    return -1;
}

int ConnectFourBoard::dropPiece(int c, Player player) {
    int r = lowestEmptyRow(c);
    if (r == -1) return -1;
    m_grid[r][c] = player;
    m_pieceCount++;
    return r;
}

void ConnectFourBoard::removePiece(int r, int c) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
        if (m_grid[r][c] != Player::None) {
            m_grid[r][c] = Player::None;
            m_pieceCount--;
        }
    }
}

std::vector<int> ConnectFourBoard::legalColumns() const {
    std::vector<int> cols;
    // Order from center outward (3, 2, 4, 1, 5, 0, 6) for better alpha-beta pruning
    static const int order[COLS] = {3, 2, 4, 1, 5, 0, 6};
    for (int c : order) {
        if (!isColumnFull(c)) {
            cols.push_back(c);
        }
    }
    return cols;
}

bool ConnectFourBoard::isFull() const {
    return m_pieceCount >= ROWS * COLS;
}

bool ConnectFourBoard::checkWin(Player player) const {
    return !getWinningLine(player).empty();
}

std::vector<std::pair<int, int>> ConnectFourBoard::getWinningLine(Player player) const {
    if (player == Player::None) return {};

    // Directions: horizontal (0, 1), vertical (1, 0), diagonal up-right (1, 1), diagonal down-right (-1, 1)
    static const int dirs[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {-1, 1}
    };

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (m_grid[r][c] != player) continue;

            for (const auto& d : dirs) {
                int dr = d[0];
                int dc = d[1];

                int endR = r + 3 * dr;
                int endC = c + 3 * dc;

                if (endR >= 0 && endR < ROWS && endC >= 0 && endC < COLS) {
                    if (m_grid[r + 1 * dr][c + 1 * dc] == player &&
                        m_grid[r + 2 * dr][c + 2 * dc] == player &&
                        m_grid[endR][endC] == player) {
                        return {
                            {r, c},
                            {r + 1 * dr, c + 1 * dc},
                            {r + 2 * dr, c + 2 * dc},
                            {endR, endC}
                        };
                    }
                }
            }
        }
    }

    return {};
}

} // namespace Strategix::ConnectFour
