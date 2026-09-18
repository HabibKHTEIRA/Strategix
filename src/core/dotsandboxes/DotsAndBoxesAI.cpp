#include "DotsAndBoxesAI.h"
#include <algorithm>
#include <random>

namespace Strategix::DotsAndBoxes {

DotsAndBoxesAI::DotsAndBoxesAI(AIDifficulty diff)
    : m_difficulty(diff)
{
}

std::optional<Edge> DotsAndBoxesAI::chooseEdge(const DotsAndBoxesBoard& board, Player /*aiPlayer*/) {
    auto available = board.getAvailableEdges();
    if (available.empty()) return std::nullopt;

    // 1. Any edge that completes a box?
    std::vector<Edge> capturingEdges;
    for (const auto& edge : available) {
        if (board.countCompletedBoxesIfDrawn(edge) > 0) {
            capturingEdges.push_back(edge);
        }
    }

    if (!capturingEdges.empty()) {
        // Double boxes completion is highest priority
        for (const auto& edge : capturingEdges) {
            if (board.countCompletedBoxesIfDrawn(edge) == 2) {
                return edge;
            }
        }
        return capturingEdges[0];
    }

    if (m_difficulty == AIDifficulty::Easy) {
        static std::mt19937 rng(1234);
        std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
        return available[dist(rng)];
    }

    // 2. Safe edges: edges that DO NOT leave a box with 3 sides
    std::vector<Edge> safeEdges;
    for (const auto& edge : available) {
        if (!board.wouldCreateThreeEdgeBox(edge)) {
            safeEdges.push_back(edge);
        }
    }

    if (!safeEdges.empty()) {
        static std::mt19937 rng(5678);
        std::uniform_int_distribution<size_t> dist(0, safeEdges.size() - 1);
        return safeEdges[dist(rng)];
    }

    // 3. Forced to give away boxes: pick the edge that gives away the minimum
    static std::mt19937 rng(9999);
    std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
    return available[dist(rng)];
}

} // namespace Strategix::DotsAndBoxes
