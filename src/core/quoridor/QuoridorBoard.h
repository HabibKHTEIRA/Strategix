#ifndef STRATEGIX_QUORIDORBOARD_H
#define STRATEGIX_QUORIDORBOARD_H

#include <vector>
#include <array>
#include <optional>

namespace Strategix::Quoridor {

enum class PlayerId {
    Player1 = 1, // Starts bottom (row 8), moves to top (row 0)
    Player2 = 2  // Starts top (row 0), moves to bottom (row 8)
};

inline PlayerId opponent(PlayerId p) {
    return (p == PlayerId::Player1) ? PlayerId::Player2 : PlayerId::Player1;
}

enum class WallOrientation {
    Horizontal,
    Vertical
};

struct Wall {
    int row; // 0..7
    int col; // 0..7
    WallOrientation orientation;

    bool operator==(const Wall& other) const {
        return row == other.row && col == other.col && orientation == other.orientation;
    }
    bool operator!=(const Wall& other) const {
        return !(*this == other);
    }
};

struct Position {
    int row; // 0..8
    int col; // 0..8

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

class QuoridorBoard {
public:
    static constexpr int SIZE = 9;

    QuoridorBoard();

    void reset();

    Position playerPos(PlayerId p) const { return (p == PlayerId::Player1) ? m_p1Pos : m_p2Pos; }
    void setPlayerPos(PlayerId p, Position pos);

    const std::vector<Wall>& walls() const { return m_walls; }

    // Check if there is a wall blocking the edge between adjacent cells (r1, c1) and (r2, c2)
    bool isBlocked(int r1, int c1, int r2, int c2) const;

    // Pawn movement
    std::vector<Position> getLegalPawnMoves(PlayerId p) const;
    bool isValidPawnMove(PlayerId p, Position dest) const;
    void movePawn(PlayerId p, Position dest);

    // Wall placement
    bool canPlaceWall(const Wall& wall) const;
    bool placeWall(const Wall& wall);
    void removeWall(const Wall& wall);

    // Pathfinding / Validation
    int shortestPathToGoal(PlayerId p) const;
    bool hasPathToGoal(PlayerId p) const;

    bool hasWon(PlayerId p) const;

private:
    Position m_p1Pos = {8, 4};
    Position m_p2Pos = {0, 4};
    std::vector<Wall> m_walls;
};

} // namespace Strategix::Quoridor

#endif // STRATEGIX_QUORIDORBOARD_H
