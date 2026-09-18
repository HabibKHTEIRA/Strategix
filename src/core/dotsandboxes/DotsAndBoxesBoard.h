#ifndef STRATEGIX_DOTSANDBOXESBOARD_H
#define STRATEGIX_DOTSANDBOXESBOARD_H

#include <vector>
#include <array>

namespace Strategix::DotsAndBoxes {

enum class Player {
    None = 0,
    Player1 = 1, // Blue
    Player2 = 2  // Red
};

inline Player opponent(Player p) {
    if (p == Player::Player1) return Player::Player2;
    if (p == Player::Player2) return Player::Player1;
    return Player::None;
}

enum class EdgeType {
    Horizontal,
    Vertical
};

struct Edge {
    EdgeType type;
    int row;
    int col;

    bool operator==(const Edge& other) const {
        return type == other.type && row == other.row && col == other.col;
    }
    bool operator!=(const Edge& other) const {
        return !(*this == other);
    }
};

class DotsAndBoxesBoard {
public:
    static constexpr int DOT_ROWS = 5;
    static constexpr int DOT_COLS = 5;
    static constexpr int BOX_ROWS = 4;
    static constexpr int BOX_COLS = 4;

    DotsAndBoxesBoard();

    void reset();

    bool isEdgeDrawn(EdgeType type, int r, int c) const;
    bool isEdgeDrawn(const Edge& edge) const { return isEdgeDrawn(edge.type, edge.row, edge.col); }

    // Returns number of boxes completed (0, 1, or 2) by drawing this edge
    int drawEdge(const Edge& edge, Player player);

    Player boxOwner(int r, int c) const;
    int boxDrawnEdgesCount(int r, int c) const;

    std::vector<Edge> getAvailableEdges() const;

    bool isFull() const;
    int score(Player p) const;

    // Checks how many boxes would be completed by drawing edge
    int countCompletedBoxesIfDrawn(const Edge& edge) const;

    // Checks if drawing edge would create a box with 3 edges (giving it to opponent)
    bool wouldCreateThreeEdgeBox(const Edge& edge) const;

private:
    std::array<std::array<bool, DOT_COLS - 1>, DOT_ROWS> m_hEdges; // 5 x 4
    std::array<std::array<bool, DOT_COLS>, DOT_ROWS - 1> m_vEdges; // 4 x 5
    std::array<std::array<Player, BOX_COLS>, BOX_ROWS> m_boxes;   // 4 x 4
};

} // namespace Strategix::DotsAndBoxes

#endif // STRATEGIX_DOTSANDBOXESBOARD_H
