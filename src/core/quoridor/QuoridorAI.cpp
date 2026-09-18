#include "QuoridorAI.h"
#include <algorithm>
#include <random>
#include <climits>

namespace Strategix::Quoridor {

QuoridorAI::QuoridorAI(AIDifficulty diff)
    : m_difficulty(diff)
{
}

int QuoridorAI::evaluate(const QuoridorBoard& board, PlayerId aiPlayer) const {
    PlayerId opp = opponent(aiPlayer);
    int myDist = board.shortestPathToGoal(aiPlayer);
    int oppDist = board.shortestPathToGoal(opp);

    if (myDist == 0) return 10000;
    if (oppDist == 0) return -10000;

    return (oppDist - myDist) * 10;
}

std::vector<Wall> QuoridorAI::getCandidateWalls(const QuoridorBoard& board, PlayerId aiPlayer) const {
    PlayerId opp = opponent(aiPlayer);
    Position oppPos = board.playerPos(opp);

    std::vector<Wall> candidates;
    // Generate walls in the vicinity of opponent pawn (+/- 2 rows and cols)
    for (int r = std::max(0, oppPos.row - 2); r <= std::min(7, oppPos.row + 2); ++r) {
        for (int c = std::max(0, oppPos.col - 2); c <= std::min(7, oppPos.col + 2); ++c) {
            Wall hw{r, c, WallOrientation::Horizontal};
            if (board.canPlaceWall(hw)) candidates.push_back(hw);

            Wall vw{r, c, WallOrientation::Vertical};
            if (board.canPlaceWall(vw)) candidates.push_back(vw);
        }
    }

    // Shuffle and pick a small subset to keep AI fast and responsive
    static std::mt19937 rng(42);
    std::shuffle(candidates.begin(), candidates.end(), rng);
    if (candidates.size() > 12) {
        candidates.resize(12);
    }

    return candidates;
}

AIAction QuoridorAI::chooseAction(const QuoridorBoard& board, PlayerId aiPlayer, int aiWallsLeft) {
    auto legalMoves = board.getLegalPawnMoves(aiPlayer);

    // 1. Check if pawn move wins immediately
    for (const auto& pos : legalMoves) {
        if ((aiPlayer == PlayerId::Player1 && pos.row == 0) ||
            (aiPlayer == PlayerId::Player2 && pos.row == 8)) {
            return AIAction{AIAction::Type::MovePawn, pos, Wall{}};
        }
    }

    // Default best move: pawn move with minimum distance to goal
    Position bestPawnMove = legalMoves[0];
    int bestPawnDist = 999;
    for (const auto& pos : legalMoves) {
        QuoridorBoard testBoard = board;
        testBoard.setPlayerPos(aiPlayer, pos);
        int d = testBoard.shortestPathToGoal(aiPlayer);
        if (d != -1 && d < bestPawnDist) {
            bestPawnDist = d;
            bestPawnMove = pos;
        }
    }

    if (m_difficulty == AIDifficulty::Easy || aiWallsLeft <= 0) {
        return AIAction{AIAction::Type::MovePawn, bestPawnMove, Wall{}};
    }

    // Evaluate pawn moves
    int bestScore = INT_MIN;
    AIAction bestAction{AIAction::Type::MovePawn, bestPawnMove, Wall{}};

    for (const auto& pos : legalMoves) {
        QuoridorBoard testBoard = board;
        testBoard.setPlayerPos(aiPlayer, pos);
        int score = evaluate(testBoard, aiPlayer);
        if (score > bestScore) {
            bestScore = score;
            bestAction = AIAction{AIAction::Type::MovePawn, pos, Wall{}};
        }
    }

    // Consider candidate walls if opponent is ahead or close
    PlayerId opp = opponent(aiPlayer);
    int oppDist = board.shortestPathToGoal(opp);
    int myDist = board.shortestPathToGoal(aiPlayer);

    if (oppDist <= myDist || m_difficulty == AIDifficulty::Hard) {
        auto walls = getCandidateWalls(board, aiPlayer);
        for (const auto& w : walls) {
            QuoridorBoard testBoard = board;
            testBoard.placeWall(w);
            int score = evaluate(testBoard, aiPlayer);
            // Bias slightly against wasting walls unless significant gain
            if (score > bestScore + 8) {
                bestScore = score;
                bestAction = AIAction{AIAction::Type::PlaceWall, Position{}, w};
            }
        }
    }

    return bestAction;
}

} // namespace Strategix::Quoridor
