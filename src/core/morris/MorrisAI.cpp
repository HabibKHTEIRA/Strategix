#include "MorrisAI.h"
#include <algorithm>
#include <random>

namespace Strategix::Morris {

MorrisAI::MorrisAI(AIDifficulty diff)
    : m_difficulty(diff)
{
}

int MorrisAI::choosePlacement(const MorrisBoard& board, Piece aiColor) {
    auto legal = board.getLegalPlacements();
    if (legal.empty()) return -1;

    Piece oppColor = opponent(aiColor);

    // 1. Can we form a mill?
    for (int pt : legal) {
        if (board.createsMill(-1, pt, aiColor)) {
            return pt;
        }
    }

    // 2. Can opponent form a mill? Block them!
    if (m_difficulty != AIDifficulty::Easy) {
        for (int pt : legal) {
            if (board.createsMill(-1, pt, oppColor)) {
                return pt;
            }
        }
    }

    // 3. Heuristic: Prefer midpoints (1, 3, 5, 7, 9, 11, 13, 15) which have higher connectivity
    static const int PREFERRED[] = {9, 11, 13, 15, 1, 3, 5, 7, 4, 12, 20, 0, 8, 16};
    for (int p : PREFERRED) {
        if (std::find(legal.begin(), legal.end(), p) != legal.end()) {
            return p;
        }
    }

    static std::mt19937 rng(777);
    std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
    return legal[dist(rng)];
}

Move MorrisAI::chooseMove(const MorrisBoard& board, Piece aiColor, bool isFlying) {
    auto legal = board.getLegalMoves(aiColor, isFlying);
    if (legal.empty()) return Move{-1, -1};

    Piece oppColor = opponent(aiColor);

    // 1. Can we form a mill?
    for (const auto& mv : legal) {
        if (board.createsMill(mv.from, mv.to, aiColor)) {
            return mv;
        }
    }

    // 2. Medium/Hard: Can opponent form a mill? Try moving to block that target square if possible
    if (m_difficulty != AIDifficulty::Easy) {
        auto oppLegal = board.getLegalMoves(oppColor, false);
        for (const auto& oppMv : oppLegal) {
            if (board.createsMill(oppMv.from, oppMv.to, oppColor)) {
                // Check if any of our legal moves ends at oppMv.to
                for (const auto& myMv : legal) {
                    if (myMv.to == oppMv.to) return myMv;
                }
            }
        }
    }

    // Fallback: pick move that maximizes remaining connectivity or random
    static std::mt19937 rng(888);
    std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
    return legal[dist(rng)];
}

int MorrisAI::chooseCapture(const MorrisBoard& board, Piece aiColor) {
    Piece oppColor = opponent(aiColor);
    auto capturable = board.getCapturablePieces(oppColor);
    if (capturable.empty()) return -1;

    // Try to capture an opponent piece that is closest to forming a mill (2 in a row)
    if (m_difficulty != AIDifficulty::Easy) {
        for (int pt : capturable) {
            for (const auto& mill : MorrisBoard::allMills()) {
                if (mill[0] == pt || mill[1] == pt || mill[2] == pt) {
                    int countOpp = 0;
                    for (int m : mill) {
                        if (board.pieceAt(m) == oppColor) countOpp++;
                    }
                    if (countOpp == 2) {
                        return pt; // Disrupt opponent 2-in-a-row
                    }
                }
            }
        }
    }

    static std::mt19937 rng(999);
    std::uniform_int_distribution<size_t> dist(0, capturable.size() - 1);
    return capturable[dist(rng)];
}

} // namespace Strategix::Morris
