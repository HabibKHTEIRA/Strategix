#include "MorrisBoard.h"
#include <algorithm>

namespace Strategix::Morris {

static const std::vector<std::array<int, 3>> S_ALL_MILLS = {
    // Outer Ring (0..7)
    {0, 1, 2}, {2, 3, 4}, {4, 5, 6}, {6, 7, 0},
    // Middle Ring (8..15)
    {8, 9, 10}, {10, 11, 12}, {12, 13, 14}, {14, 15, 8},
    // Inner Ring (16..23)
    {16, 17, 18}, {18, 19, 20}, {20, 21, 22}, {22, 23, 16},
    // Cross connections at midpoints
    {1, 9, 17},   // Top mid
    {3, 11, 19},  // Right mid
    {5, 13, 21},  // Bottom mid
    {7, 15, 23}   // Left mid
};

const std::vector<std::array<int, 3>>& MorrisBoard::allMills() {
    return S_ALL_MILLS;
}

const std::vector<int>& MorrisBoard::adjacentPoints(int pt) {
    static const std::vector<std::vector<int>> ADJ = {
        /*  0 */ {1, 7},
        /*  1 */ {0, 2, 9},
        /*  2 */ {1, 3},
        /*  3 */ {2, 4, 11},
        /*  4 */ {3, 5},
        /*  5 */ {4, 6, 13},
        /*  6 */ {5, 7},
        /*  7 */ {6, 0, 15},

        /*  8 */ {9, 15},
        /*  9 */ {8, 10, 1, 17},
        /* 10 */ {9, 11},
        /* 11 */ {10, 12, 3, 19},
        /* 12 */ {11, 13},
        /* 13 */ {12, 14, 5, 21},
        /* 14 */ {13, 15},
        /* 15 */ {14, 8, 7, 23},

        /* 16 */ {17, 23},
        /* 17 */ {16, 18, 9},
        /* 18 */ {17, 19},
        /* 19 */ {18, 20, 11},
        /* 20 */ {19, 21},
        /* 21 */ {20, 22, 13},
        /* 22 */ {21, 23},
        /* 23 */ {22, 16, 15}
    };

    if (pt >= 0 && pt < 24) return ADJ[pt];
    static const std::vector<int> EMPTY;
    return EMPTY;
}

MorrisBoard::MorrisBoard() {
    reset();
}

void MorrisBoard::reset() {
    m_points.fill(Piece::Empty);
}

Piece MorrisBoard::pieceAt(int pt) const {
    if (pt < 0 || pt >= NUM_POINTS) return Piece::Empty;
    return m_points[pt];
}

void MorrisBoard::setPiece(int pt, Piece p) {
    if (pt >= 0 && pt < NUM_POINTS) {
        m_points[pt] = p;
    }
}

bool MorrisBoard::isPartOfMill(int pt, Piece player) const {
    if (m_points[pt] != player) return false;
    for (const auto& mill : S_ALL_MILLS) {
        if (mill[0] == pt || mill[1] == pt || mill[2] == pt) {
            if (m_points[mill[0]] == player &&
                m_points[mill[1]] == player &&
                m_points[mill[2]] == player) {
                return true;
            }
        }
    }
    return false;
}

bool MorrisBoard::createsMill(int from, int to, Piece player) const {
    // Temporarily apply move
    MorrisBoard* self = const_cast<MorrisBoard*>(this);
    Piece oldFrom = (from >= 0) ? self->m_points[from] : Piece::Empty;
    if (from >= 0) self->m_points[from] = Piece::Empty;
    self->m_points[to] = player;

    bool result = self->isPartOfMill(to, player);

    // Revert
    self->m_points[to] = Piece::Empty;
    if (from >= 0) self->m_points[from] = oldFrom;

    return result;
}

std::vector<int> MorrisBoard::getLegalPlacements() const {
    std::vector<int> places;
    for (int i = 0; i < NUM_POINTS; ++i) {
        if (m_points[i] == Piece::Empty) {
            places.push_back(i);
        }
    }
    return places;
}

std::vector<int> MorrisBoard::getLegalMovesFrom(int from, bool isFlying) const {
    std::vector<int> dests;
    if (from < 0 || from >= NUM_POINTS) return dests;

    if (isFlying) {
        return getLegalPlacements();
    }

    for (int adj : adjacentPoints(from)) {
        if (m_points[adj] == Piece::Empty) {
            dests.push_back(adj);
        }
    }
    return dests;
}

std::vector<Move> MorrisBoard::getLegalMoves(Piece player, bool isFlying) const {
    std::vector<Move> moves;
    for (int from = 0; from < NUM_POINTS; ++from) {
        if (m_points[from] == player) {
            auto dests = getLegalMovesFrom(from, isFlying);
            for (int to : dests) {
                moves.push_back(Move{from, to});
            }
        }
    }
    return moves;
}

std::vector<int> MorrisBoard::getCapturablePieces(Piece opponentPiece) const {
    std::vector<int> nonMillPieces;
    std::vector<int> allOppPieces;

    for (int i = 0; i < NUM_POINTS; ++i) {
        if (m_points[i] == opponentPiece) {
            allOppPieces.push_back(i);
            if (!isPartOfMill(i, opponentPiece)) {
                nonMillPieces.push_back(i);
            }
        }
    }

    // Rule: cannot remove a piece from an opponent mill unless ALL opponent pieces are in mills
    if (!nonMillPieces.empty()) {
        return nonMillPieces;
    }
    return allOppPieces;
}

int MorrisBoard::pieceCount(Piece p) const {
    int c = 0;
    for (int i = 0; i < NUM_POINTS; ++i) {
        if (m_points[i] == p) c++;
    }
    return c;
}

} // namespace Strategix::Morris
