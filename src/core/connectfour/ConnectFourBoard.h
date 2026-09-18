#ifndef STRATEGIX_CONNECTFOURBOARD_H
#define STRATEGIX_CONNECTFOURBOARD_H

#include <vector>
#include <array>
#include <optional>

namespace Strategix::ConnectFour {

enum class Player {
    None = 0,
    Red = 1,
    Yellow = 2
};

inline Player opponent(Player p) {
    if (p == Player::Red) return Player::Yellow;
    if (p == Player::Yellow) return Player::Red;
    return Player::None;
}

class ConnectFourBoard {
public:
    static constexpr int ROWS = 6;
    static constexpr int COLS = 7;

    ConnectFourBoard();

    void clear();

    Player cell(int r, int c) const;
    bool isColumnFull(int c) const;
    int lowestEmptyRow(int c) const;

    // Drop a piece into column c (0..6). Returns the row where it landed, or -1 if full.
    int dropPiece(int c, Player player);

    // Undo a drop at column c, row r
    void removePiece(int r, int c);

    std::vector<int> legalColumns() const;

    bool checkWin(Player player) const;
    bool isFull() const;

    // Returns the 4 winning coordinates if player has won, otherwise empty
    std::vector<std::pair<int, int>> getWinningLine(Player player) const;

private:
    std::array<std::array<Player, COLS>, ROWS> m_grid;
    int m_pieceCount = 0;
};

} // namespace Strategix::ConnectFour

#endif // STRATEGIX_CONNECTFOURBOARD_H
