#ifndef STRATEGIX_GOBOARD_H
#define STRATEGIX_GOBOARD_H

#include <vector>
#include <set>
#include <array>
#include <optional>

namespace Strategix::Go {

enum class StoneColor { None, Black, White };

inline StoneColor opponentColor(StoneColor c) {
    if (c == StoneColor::Black) return StoneColor::White;
    if (c == StoneColor::White) return StoneColor::Black;
    return StoneColor::None;
}

struct Point {
    int row = 0;
    int col = 0;

    constexpr bool operator==(const Point& o) const { return row == o.row && col == o.col; }
    constexpr bool operator!=(const Point& o) const { return !(*this == o); }
    constexpr bool operator<(const Point& o) const {
        return row < o.row || (row == o.row && col < o.col);
    }
};

struct Group {
    StoneColor color = StoneColor::None;
    std::set<Point> stones;
    std::set<Point> liberties;
};

class GoBoard {
public:
    static constexpr int DEFAULT_SIZE = 9;

    explicit GoBoard(int size = DEFAULT_SIZE);

    void reset();

    int size() const { return m_size; }
    bool isValid(int r, int c) const { return r >= 0 && r < m_size && c >= 0 && c < m_size; }
    bool isValid(Point p) const { return isValid(p.row, p.col); }

    StoneColor stoneAt(int r, int c) const;
    StoneColor stoneAt(Point p) const { return stoneAt(p.row, p.col); }

    void setStone(int r, int c, StoneColor color);
    void setStone(Point p, StoneColor color) { setStone(p.row, p.col, color); }

    std::vector<Point> adjacentPoints(Point p) const;

    // Returns the connected group of stones containing p, and its liberties
    Group getGroup(Point p) const;

    // Checks if placing a stone at p for color is legal (considers captures, suicide, and ko)
    bool isLegalMove(Point p, StoneColor color, const std::optional<Point>& koPoint) const;

    // Plays move, captures dead opponent stones, updates board. Returns list of captured stones.
    std::vector<Point> playMove(Point p, StoneColor color);

    int countStones(StoneColor color) const;

    // Calculate score / territory using flood-fill territory detection
    std::pair<int, int> countTerritory() const; // <BlackScore, WhiteScore>

    // Equality for Ko check
    bool operator==(const GoBoard& o) const;

private:
    int m_size = DEFAULT_SIZE;
    std::vector<std::vector<StoneColor>> m_grid;
};

} // namespace Strategix::Go

#endif // STRATEGIX_GOBOARD_H
