#ifndef STRATEGIX_REVERSIBOARD_H
#define STRATEGIX_REVERSIBOARD_H

#include <vector>
#include <array>

namespace Strategix::Reversi {

enum class Piece {
    Empty = 0,
    Black = 1,
    White = 2
};

inline Piece opponent(Piece p) {
    if (p == Piece::Black) return Piece::White;
    if (p == Piece::White) return Piece::Black;
    return Piece::Empty;
}

struct Move {
    int row;
    int col;
    bool operator==(const Move& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Move& other) const {
        return !(*this == other);
    }
};

class ReversiBoard {
public:
    static constexpr int SIZE = 8;

    ReversiBoard();

    void reset();

    Piece pieceAt(int r, int c) const;
    void setPiece(int r, int c, Piece p);

    bool isValidMove(int r, int c, Piece player) const;
    std::vector<Move> getLegalMoves(Piece player) const;

    // Applies move and flips trapped pieces. Returns number of pieces flipped.
    int applyMove(int r, int c, Piece player);

    int blackCount() const;
    int whiteCount() const;
    int emptyCount() const;

    bool isGameOver() const;

    // Get list of pieces that would be flipped by playing at (r, c)
    std::vector<std::pair<int, int>> getFlippablePieces(int r, int c, Piece player) const;

private:
    std::array<std::array<Piece, SIZE>, SIZE> m_grid;
};

} // namespace Strategix::Reversi

#endif // STRATEGIX_REVERSIBOARD_H
