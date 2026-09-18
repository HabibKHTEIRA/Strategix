#include "QuoridorBoard.h"
#include <queue>
#include <algorithm>

namespace Strategix::Quoridor {

QuoridorBoard::QuoridorBoard() {
    reset();
}

void QuoridorBoard::reset() {
    m_p1Pos = {8, 4};
    m_p2Pos = {0, 4};
    m_walls.clear();
}

void QuoridorBoard::setPlayerPos(PlayerId p, Position pos) {
    if (p == PlayerId::Player1) m_p1Pos = pos;
    else m_p2Pos = pos;
}

bool QuoridorBoard::isBlocked(int r1, int c1, int r2, int c2) const {
    if (r1 == r2) {
        int minC = std::min(c1, c2);
        // Moving horizontally between minC and minC + 1
        for (const auto& w : m_walls) {
            if (w.orientation == WallOrientation::Vertical && w.col == minC) {
                if (w.row == r1 || w.row == r1 - 1) {
                    return true;
                }
            }
        }
    } else if (c1 == c2) {
        int minR = std::min(r1, r2);
        // Moving vertically between minR and minR + 1
        for (const auto& w : m_walls) {
            if (w.orientation == WallOrientation::Horizontal && w.row == minR) {
                if (w.col == c1 || w.col == c1 - 1) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool QuoridorBoard::canPlaceWall(const Wall& wall) const {
    if (wall.row < 0 || wall.row > 7 || wall.col < 0 || wall.col > 7) return false;

    // Check collisions with existing walls
    for (const auto& w : m_walls) {
        if (w.row == wall.row && w.col == wall.col) {
            return false; // Exactly at the same cross-point
        }
        if (wall.orientation == WallOrientation::Horizontal && w.orientation == WallOrientation::Horizontal) {
            if (w.row == wall.row && (w.col == wall.col - 1 || w.col == wall.col + 1)) {
                return false; // Overlapping horizontal wall
            }
        }
        if (wall.orientation == WallOrientation::Vertical && w.orientation == WallOrientation::Vertical) {
            if (w.col == wall.col && (w.row == wall.row - 1 || w.row == wall.row + 1)) {
                return false; // Overlapping vertical wall
            }
        }
    }

    // Path check: cannot block either player from reaching their goal line
    QuoridorBoard* nonConst = const_cast<QuoridorBoard*>(this);
    nonConst->m_walls.push_back(wall);

    bool p1Ok = nonConst->hasPathToGoal(PlayerId::Player1);
    bool p2Ok = nonConst->hasPathToGoal(PlayerId::Player2);

    nonConst->m_walls.pop_back();

    return p1Ok && p2Ok;
}

bool QuoridorBoard::placeWall(const Wall& wall) {
    if (!canPlaceWall(wall)) return false;
    m_walls.push_back(wall);
    return true;
}

void QuoridorBoard::removeWall(const Wall& wall) {
    auto it = std::find(m_walls.begin(), m_walls.end(), wall);
    if (it != m_walls.end()) {
        m_walls.erase(it);
    }
}

int QuoridorBoard::shortestPathToGoal(PlayerId p) const {
    Position start = playerPos(p);
    int targetRow = (p == PlayerId::Player1) ? 0 : 8;

    if (start.row == targetRow) return 0;

    std::array<std::array<int, SIZE>, SIZE> dist;
    for (auto& row : dist) row.fill(-1);

    std::queue<Position> q;
    q.push(start);
    dist[start.row][start.col] = 0;

    static const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (!q.empty()) {
        Position curr = q.front();
        q.pop();

        if (curr.row == targetRow) {
            return dist[curr.row][curr.col];
        }

        for (const auto& d : dirs) {
            int nr = curr.row + d[0];
            int nc = curr.col + d[1];

            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                if (!isBlocked(curr.row, curr.col, nr, nc) && dist[nr][nc] == -1) {
                    dist[nr][nc] = dist[curr.row][curr.col] + 1;
                    q.push({nr, nc});
                }
            }
        }
    }

    return -1; // No path
}

bool QuoridorBoard::hasPathToGoal(PlayerId p) const {
    return shortestPathToGoal(p) != -1;
}

std::vector<Position> QuoridorBoard::getLegalPawnMoves(PlayerId p) const {
    std::vector<Position> moves;
    Position myPos = playerPos(p);
    Position oppPos = playerPos(opponent(p));

    static const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (const auto& d : dirs) {
        int nr = myPos.row + d[0];
        int nc = myPos.col + d[1];

        if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
        if (isBlocked(myPos.row, myPos.col, nr, nc)) continue;

        if (Position{nr, nc} != oppPos) {
            moves.push_back({nr, nc});
        } else {
            // Opponent is adjacent! Try straight jump
            int jnr = nr + d[0];
            int jnc = nc + d[1];
            bool canJumpStraight = false;

            if (jnr >= 0 && jnr < SIZE && jnc >= 0 && jnc < SIZE) {
                if (!isBlocked(nr, nc, jnr, jnc)) {
                    moves.push_back({jnr, jnc});
                    canJumpStraight = true;
                }
            }

            // If cannot jump straight (wall or edge behind opponent), can jump diagonally
            if (!canJumpStraight) {
                // Diagonals perpendicular to d
                int perpDirs[2][2];
                if (d[0] != 0) { // Vertical jump blocked -> check left & right
                    perpDirs[0][0] = 0; perpDirs[0][1] = -1;
                    perpDirs[1][0] = 0; perpDirs[1][1] = 1;
                } else { // Horizontal jump blocked -> check up & down
                    perpDirs[0][0] = -1; perpDirs[0][1] = 0;
                    perpDirs[1][0] = 1;  perpDirs[1][1] = 0;
                }

                for (const auto& pd : perpDirs) {
                    int diagR = nr + pd[0];
                    int diagC = nc + pd[1];
                    if (diagR >= 0 && diagR < SIZE && diagC >= 0 && diagC < SIZE) {
                        if (!isBlocked(nr, nc, diagR, diagC)) {
                            moves.push_back({diagR, diagC});
                        }
                    }
                }
            }
        }
    }

    return moves;
}

bool QuoridorBoard::isValidPawnMove(PlayerId p, Position dest) const {
    auto legal = getLegalPawnMoves(p);
    return std::find(legal.begin(), legal.end(), dest) != legal.end();
}

void QuoridorBoard::movePawn(PlayerId p, Position dest) {
    setPlayerPos(p, dest);
}

bool QuoridorBoard::hasWon(PlayerId p) const {
    if (p == PlayerId::Player1) return m_p1Pos.row == 0;
    return m_p2Pos.row == 8;
}

} // namespace Strategix::Quoridor
