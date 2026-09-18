#include "GoAI.h"
#include <chrono>
#include <algorithm>

namespace Strategix::Go {

GoAI::GoAI(AIDifficulty diff)
    : m_difficulty(diff)
    , m_rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()))
{
}

std::optional<Point> GoAI::chooseMove(const GoBoard& board, StoneColor myColor, const std::optional<Point>& koPoint) {
    int size = board.size();
    std::vector<Point> legalMoves;
    StoneColor opp = opponentColor(myColor);

    // Collect legal moves
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            Point p{r, c};
            if (board.isLegalMove(p, myColor, koPoint)) {
                legalMoves.push_back(p);
            }
        }
    }

    if (legalMoves.empty()) {
        return std::nullopt; // Must pass
    }

    if (m_difficulty == AIDifficulty::Easy) {
        std::uniform_int_distribution<size_t> dist(0, legalMoves.size() - 1);
        return legalMoves[dist(m_rng)];
    }

    // Medium & Hard heuristic search:
    std::vector<std::pair<Point, int>> scoredMoves;

    for (Point p : legalMoves) {
        GoBoard copy = board;
        auto captured = copy.playMove(p, myColor);
        int score = 0;

        // 1. Immediate capture reward
        if (!captured.empty()) {
            score += static_cast<int>(captured.size()) * 50;
        }

        // 2. Liberties of the newly played stone's group
        Group ownGroup = copy.getGroup(p);
        if (ownGroup.liberties.size() == 1) {
            // Self-atari penalty!
            score -= 60;
        } else {
            score += static_cast<int>(ownGroup.liberties.size()) * 5;
        }

        // 3. Atari detection on adjacent opponent groups
        for (Point adj : copy.adjacentPoints(p)) {
            if (copy.stoneAt(adj) == opp) {
                Group oppGroup = copy.getGroup(adj);
                if (oppGroup.liberties.size() == 1) {
                    score += 25; // Put opponent in atari!
                }
            }
        }

        // 4. Strategic positional heuristics (center/star points preference)
        int distFromCenter = std::abs(p.row - size / 2) + std::abs(p.col - size / 2);
        score += (size - distFromCenter);

        // Third line / Fourth line development bonus
        if ((p.row >= 2 && p.row <= size - 3) && (p.col >= 2 && p.col <= size - 3)) {
            score += 10;
        }

        scoredMoves.emplace_back(p, score);
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    if (scoredMoves.empty() || scoredMoves[0].second < -30) {
        // If best move is terribly harmful (e.g. filling eye / self-atari), pass
        return std::nullopt;
    }

    // Add slight variety among top 3 best moves
    size_t pickRange = std::min<size_t>(3, scoredMoves.size());
    std::uniform_int_distribution<size_t> dist(0, pickRange - 1);
    return scoredMoves[dist(m_rng)].first;
}

} // namespace Strategix::Go
