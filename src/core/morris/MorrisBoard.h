#ifndef STRATEGIX_MORRISBOARD_H
#define STRATEGIX_MORRISBOARD_H

#include <vector>
#include <array>

namespace Strategix::Morris {

enum class Piece {
    Empty = 0,
    White = 1,
    Black = 2
};

inline Piece opponent(Piece p) {
    if (p == Piece::White) return Piece::Black;
    if (p == Piece::Black) return Piece::White;
    return Piece::Empty;
}

struct Move {
    int from; // -1 for Phase 1 (placing)
    int to;   // 0..23

    bool operator==(const Move& other) const {
        return from == other.from && to == other.to;
    }
    bool operator!=(const Move& other) const {
        return !(*this == other);
    }
};

class MorrisBoard {
public:
    static constexpr int NUM_POINTS = 24;

    MorrisBoard();

    void reset();

    Piece pieceAt(int pt) const;
    void setPiece(int pt, Piece p);

    // Adjacent points graph
    static const std::vector<int>& adjacentPoints(int pt);

    // All 16 possible mills
    static const std::vector<std::array<int, 3>>& allMills();

    // Check if point pt is currently part of any completed mill for player
    bool isPartOfMill(int pt, Piece player) const;

    // Check if move (from, to) creates a new mill for player
    bool createsMill(int from, int to, Piece player) const;

    // Legal placements (empty points)
    std::vector<int> getLegalPlacements() const;

    // Legal moves for a specific point
    std::vector<int> getLegalMovesFrom(int from, bool isFlying) const;

    // All legal moves for player in Phase 2/3
    std::vector<Move> getLegalMoves(Piece player, bool isFlying) const;

    // Capturable opponent pieces (cannot capture piece in mill unless all opponent pieces are in mills)
    std::vector<int> getCapturablePieces(Piece opponentPiece) const;

    int pieceCount(Piece p) const;

private:
    std::array<Piece, NUM_POINTS> m_points;
};

} // namespace Strategix::Morris

#endif // STRATEGIX_MORRISBOARD_H
