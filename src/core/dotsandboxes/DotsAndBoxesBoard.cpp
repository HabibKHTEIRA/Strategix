#include "DotsAndBoxesBoard.h"

namespace Strategix::DotsAndBoxes {

DotsAndBoxesBoard::DotsAndBoxesBoard() {
    reset();
}

void DotsAndBoxesBoard::reset() {
    for (auto& row : m_hEdges) row.fill(false);
    for (auto& row : m_vEdges) row.fill(false);
    for (auto& row : m_boxes) row.fill(Player::None);
}

bool DotsAndBoxesBoard::isEdgeDrawn(EdgeType type, int r, int c) const {
    if (type == EdgeType::Horizontal) {
        if (r < 0 || r >= DOT_ROWS || c < 0 || c >= DOT_COLS - 1) return true;
        return m_hEdges[r][c];
    } else {
        if (r < 0 || r >= DOT_ROWS - 1 || c < 0 || c >= DOT_COLS) return true;
        return m_vEdges[r][c];
    }
}

Player DotsAndBoxesBoard::boxOwner(int r, int c) const {
    if (r < 0 || r >= BOX_ROWS || c < 0 || c >= BOX_COLS) return Player::None;
    return m_boxes[r][c];
}

int DotsAndBoxesBoard::boxDrawnEdgesCount(int r, int c) const {
    if (r < 0 || r >= BOX_ROWS || c < 0 || c >= BOX_COLS) return 0;
    int count = 0;
    if (m_hEdges[r][c]) count++;       // Top
    if (m_hEdges[r + 1][c]) count++;   // Bottom
    if (m_vEdges[r][c]) count++;       // Left
    if (m_vEdges[r][c + 1]) count++;   // Right
    return count;
}

int DotsAndBoxesBoard::drawEdge(const Edge& edge, Player player) {
    if (isEdgeDrawn(edge)) return 0;

    if (edge.type == EdgeType::Horizontal) {
        m_hEdges[edge.row][edge.col] = true;
    } else {
        m_vEdges[edge.row][edge.col] = true;
    }

    int completed = 0;

    // Check affected boxes
    if (edge.type == EdgeType::Horizontal) {
        // Box above: (row - 1, col)
        if (edge.row > 0) {
            int br = edge.row - 1;
            int bc = edge.col;
            if (boxDrawnEdgesCount(br, bc) == 4 && m_boxes[br][bc] == Player::None) {
                m_boxes[br][bc] = player;
                completed++;
            }
        }
        // Box below: (row, col)
        if (edge.row < BOX_ROWS) {
            int br = edge.row;
            int bc = edge.col;
            if (boxDrawnEdgesCount(br, bc) == 4 && m_boxes[br][bc] == Player::None) {
                m_boxes[br][bc] = player;
                completed++;
            }
        }
    } else {
        // Box to the left: (row, col - 1)
        if (edge.col > 0) {
            int br = edge.row;
            int bc = edge.col - 1;
            if (boxDrawnEdgesCount(br, bc) == 4 && m_boxes[br][bc] == Player::None) {
                m_boxes[br][bc] = player;
                completed++;
            }
        }
        // Box to the right: (row, col)
        if (edge.col < BOX_COLS) {
            int br = edge.row;
            int bc = edge.col;
            if (boxDrawnEdgesCount(br, bc) == 4 && m_boxes[br][bc] == Player::None) {
                m_boxes[br][bc] = player;
                completed++;
            }
        }
    }

    return completed;
}

int DotsAndBoxesBoard::countCompletedBoxesIfDrawn(const Edge& edge) const {
    if (isEdgeDrawn(edge)) return 0;

    int completed = 0;
    if (edge.type == EdgeType::Horizontal) {
        if (edge.row > 0 && boxDrawnEdgesCount(edge.row - 1, edge.col) == 3) completed++;
        if (edge.row < BOX_ROWS && boxDrawnEdgesCount(edge.row, edge.col) == 3) completed++;
    } else {
        if (edge.col > 0 && boxDrawnEdgesCount(edge.row, edge.col - 1) == 3) completed++;
        if (edge.col < BOX_COLS && boxDrawnEdgesCount(edge.row, edge.col) == 3) completed++;
    }
    return completed;
}

bool DotsAndBoxesBoard::wouldCreateThreeEdgeBox(const Edge& edge) const {
    if (isEdgeDrawn(edge)) return false;

    if (edge.type == EdgeType::Horizontal) {
        if (edge.row > 0 && boxDrawnEdgesCount(edge.row - 1, edge.col) == 2) return true;
        if (edge.row < BOX_ROWS && boxDrawnEdgesCount(edge.row, edge.col) == 2) return true;
    } else {
        if (edge.col > 0 && boxDrawnEdgesCount(edge.row, edge.col - 1) == 2) return true;
        if (edge.col < BOX_COLS && boxDrawnEdgesCount(edge.row, edge.col) == 2) return true;
    }
    return false;
}

std::vector<Edge> DotsAndBoxesBoard::getAvailableEdges() const {
    std::vector<Edge> edges;
    for (int r = 0; r < DOT_ROWS; ++r) {
        for (int c = 0; c < DOT_COLS - 1; ++c) {
            if (!m_hEdges[r][c]) edges.push_back({EdgeType::Horizontal, r, c});
        }
    }
    for (int r = 0; r < DOT_ROWS - 1; ++r) {
        for (int c = 0; c < DOT_COLS; ++c) {
            if (!m_vEdges[r][c]) edges.push_back({EdgeType::Vertical, r, c});
        }
    }
    return edges;
}

bool DotsAndBoxesBoard::isFull() const {
    return score(Player::Player1) + score(Player::Player2) == BOX_ROWS * BOX_COLS;
}

int DotsAndBoxesBoard::score(Player p) const {
    int sc = 0;
    for (int r = 0; r < BOX_ROWS; ++r) {
        for (int c = 0; c < BOX_COLS; ++c) {
            if (m_boxes[r][c] == p) sc++;
        }
    }
    return sc;
}

} // namespace Strategix::DotsAndBoxes
