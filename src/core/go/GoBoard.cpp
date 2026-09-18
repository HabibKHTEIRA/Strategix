#include "GoBoard.h"
#include <queue>
#include <set>

namespace Strategix::Go {

GoBoard::GoBoard(int size)
    : m_size(size)
    , m_grid(size, std::vector<StoneColor>(size, StoneColor::None))
{
}

void GoBoard::reset() {
    for (int r = 0; r < m_size; ++r) {
        for (int c = 0; c < m_size; ++c) {
            m_grid[r][c] = StoneColor::None;
        }
    }
}

StoneColor GoBoard::stoneAt(int r, int c) const {
    if (!isValid(r, c)) return StoneColor::None;
    return m_grid[r][c];
}

void GoBoard::setStone(int r, int c, StoneColor color) {
    if (isValid(r, c)) {
        m_grid[r][c] = color;
    }
}

std::vector<Point> GoBoard::adjacentPoints(Point p) const {
    std::vector<Point> adj;
    adj.reserve(4);
    static const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (auto d : dirs) {
        int nr = p.row + d[0];
        int nc = p.col + d[1];
        if (isValid(nr, nc)) {
            adj.push_back({nr, nc});
        }
    }
    return adj;
}

Group GoBoard::getGroup(Point p) const {
    Group grp;
    grp.color = stoneAt(p);
    if (grp.color == StoneColor::None) {
        return grp;
    }

    std::queue<Point> q;
    q.push(p);
    grp.stones.insert(p);

    while (!q.empty()) {
        Point cur = q.front();
        q.pop();

        for (Point nxt : adjacentPoints(cur)) {
            StoneColor sc = stoneAt(nxt);
            if (sc == StoneColor::None) {
                grp.liberties.insert(nxt);
            } else if (sc == grp.color) {
                if (grp.stones.find(nxt) == grp.stones.end()) {
                    grp.stones.insert(nxt);
                    q.push(nxt);
                }
            }
        }
    }

    return grp;
}

bool GoBoard::isLegalMove(Point p, StoneColor color, const std::optional<Point>& koPoint) const {
    if (!isValid(p) || stoneAt(p) != StoneColor::None) {
        return false;
    }

    if (koPoint && *koPoint == p) {
        return false; // Ko prohibition
    }

    // Simulate placement
    GoBoard temp = *this;
    temp.setStone(p, color);

    StoneColor opp = opponentColor(color);
    bool capturesOpponent = false;

    for (Point adj : temp.adjacentPoints(p)) {
        if (temp.stoneAt(adj) == opp) {
            Group oppGroup = temp.getGroup(adj);
            if (oppGroup.liberties.empty()) {
                capturesOpponent = true;
                break;
            }
        }
    }

    if (capturesOpponent) {
        return true; // Captures are always legal (unless ko, checked above)
    }

    // Check suicide: does own group have at least 1 liberty?
    Group ownGroup = temp.getGroup(p);
    return !ownGroup.liberties.empty();
}

std::vector<Point> GoBoard::playMove(Point p, StoneColor color) {
    if (!isValid(p)) return {};

    setStone(p, color);
    StoneColor opp = opponentColor(color);

    std::vector<Point> capturedStones;

    for (Point adj : adjacentPoints(p)) {
        if (stoneAt(adj) == opp) {
            Group oppGroup = getGroup(adj);
            if (oppGroup.liberties.empty()) {
                for (Point dead : oppGroup.stones) {
                    setStone(dead, StoneColor::None);
                    capturedStones.push_back(dead);
                }
            }
        }
    }

    return capturedStones;
}

int GoBoard::countStones(StoneColor color) const {
    int count = 0;
    for (int r = 0; r < m_size; ++r) {
        for (int c = 0; c < m_size; ++c) {
            if (m_grid[r][c] == color) ++count;
        }
    }
    return count;
}

std::pair<int, int> GoBoard::countTerritory() const {
    int blackTerritory = 0;
    int whiteTerritory = 0;

    std::vector<std::vector<bool>> visited(m_size, std::vector<bool>(m_size, false));

    for (int r = 0; r < m_size; ++r) {
        for (int c = 0; c < m_size; ++c) {
            if (m_grid[r][c] != StoneColor::None || visited[r][c]) {
                continue;
            }

            // Flood-fill empty territory
            std::queue<Point> q;
            std::vector<Point> territoryPoints;
            bool touchesBlack = false;
            bool touchesWhite = false;

            q.push({r, c});
            visited[r][c] = true;

            while (!q.empty()) {
                Point cur = q.front();
                q.pop();
                territoryPoints.push_back(cur);

                for (Point adj : adjacentPoints(cur)) {
                    StoneColor sc = stoneAt(adj);
                    if (sc == StoneColor::Black) touchesBlack = true;
                    else if (sc == StoneColor::White) touchesWhite = true;
                    else if (!visited[adj.row][adj.col]) {
                        visited[adj.row][adj.col] = true;
                        q.push(adj);
                    }
                }
            }

            if (touchesBlack && !touchesWhite) {
                blackTerritory += static_cast<int>(territoryPoints.size());
            } else if (touchesWhite && !touchesBlack) {
                whiteTerritory += static_cast<int>(territoryPoints.size());
            }
        }
    }

    return {blackTerritory, whiteTerritory};
}

bool GoBoard::operator==(const GoBoard& o) const {
    if (m_size != o.m_size) return false;
    return m_grid == o.m_grid;
}

} // namespace Strategix::Go
