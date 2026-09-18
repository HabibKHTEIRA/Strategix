#include "ConnectFourAI.h"
#include <algorithm>
#include <random>
#include <climits>

namespace Strategix::ConnectFour {

ConnectFourAI::ConnectFourAI(AIDifficulty diff)
    : m_difficulty(diff)
{
}

int ConnectFourAI::chooseColumn(const ConnectFourBoard& board, Player aiPlayer) {
    auto legal = board.legalColumns();
    if (legal.empty()) return -1;

    // 1. Check if AI can win immediately in 1 move
    for (int col : legal) {
        ConnectFourBoard testBoard = board;
        testBoard.dropPiece(col, aiPlayer);
        if (testBoard.checkWin(aiPlayer)) {
            return col;
        }
    }

    // 2. Check if opponent would win immediately in 1 move -> block!
    Player opp = opponent(aiPlayer);
    for (int col : legal) {
        ConnectFourBoard testBoard = board;
        testBoard.dropPiece(col, opp);
        if (testBoard.checkWin(opp)) {
            return col;
        }
    }

    if (m_difficulty == AIDifficulty::Easy) {
        // Random legal column
        static std::mt19937 rng(1337);
        std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
        return legal[dist(rng)];
    }

    int maxDepth = (m_difficulty == AIDifficulty::Medium) ? 3 : 5;

    int bestCol = legal[0];
    int bestVal = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;

    for (int col : legal) {
        ConnectFourBoard testBoard = board;
        int r = testBoard.dropPiece(col, aiPlayer);
        if (r == -1) continue;

        int score = minimax(testBoard, maxDepth - 1, alpha, beta, false, aiPlayer);
        if (score > bestVal) {
            bestVal = score;
            bestCol = col;
        }
        alpha = std::max(alpha, bestVal);
    }

    return bestCol;
}

int ConnectFourAI::minimax(ConnectFourBoard& board, int depth, int alpha, int beta, bool maximizing, Player aiPlayer) {
    Player opp = opponent(aiPlayer);

    if (board.checkWin(aiPlayer)) return 100000 + depth;
    if (board.checkWin(opp)) return -100000 - depth;
    if (board.isFull() || depth == 0) {
        return evaluateBoard(board, aiPlayer);
    }

    auto legal = board.legalColumns();

    if (maximizing) {
        int maxEval = INT_MIN;
        for (int col : legal) {
            int r = board.dropPiece(col, aiPlayer);
            int eval = minimax(board, depth - 1, alpha, beta, false, aiPlayer);
            board.removePiece(r, col);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (int col : legal) {
            int r = board.dropPiece(col, opp);
            int eval = minimax(board, depth - 1, alpha, beta, true, aiPlayer);
            board.removePiece(r, col);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

int ConnectFourAI::evaluateBoard(const ConnectFourBoard& board, Player aiPlayer) const {
    int score = 0;
    Player opp = opponent(aiPlayer);

    // Center column preference (center pieces give vastly more connect opportunities)
    int centerCount = 0;
    for (int r = 0; r < ConnectFourBoard::ROWS; ++r) {
        if (board.cell(r, 3) == aiPlayer) centerCount++;
        else if (board.cell(r, 3) == opp) centerCount--;
    }
    score += centerCount * 6;

    // Horizontal windows
    for (int r = 0; r < ConnectFourBoard::ROWS; ++r) {
        for (int c = 0; c < ConnectFourBoard::COLS - 3; ++c) {
            std::array<Player, 4> win = {board.cell(r, c), board.cell(r, c+1), board.cell(r, c+2), board.cell(r, c+3)};
            score += evaluateWindow(win, aiPlayer);
        }
    }

    // Vertical windows
    for (int c = 0; c < ConnectFourBoard::COLS; ++c) {
        for (int r = 0; r < ConnectFourBoard::ROWS - 3; ++r) {
            std::array<Player, 4> win = {board.cell(r, c), board.cell(r+1, c), board.cell(r+2, c), board.cell(r+3, c)};
            score += evaluateWindow(win, aiPlayer);
        }
    }

    // Positive diagonal windows
    for (int r = 0; r < ConnectFourBoard::ROWS - 3; ++r) {
        for (int c = 0; c < ConnectFourBoard::COLS - 3; ++c) {
            std::array<Player, 4> win = {board.cell(r, c), board.cell(r+1, c+1), board.cell(r+2, c+2), board.cell(r+3, c+3)};
            score += evaluateWindow(win, aiPlayer);
        }
    }

    // Negative diagonal windows
    for (int r = 3; r < ConnectFourBoard::ROWS; ++r) {
        for (int c = 0; c < ConnectFourBoard::COLS - 3; ++c) {
            std::array<Player, 4> win = {board.cell(r, c), board.cell(r-1, c+1), board.cell(r-2, c+2), board.cell(r-3, c+3)};
            score += evaluateWindow(win, aiPlayer);
        }
    }

    return score;
}

int ConnectFourAI::evaluateWindow(const std::array<Player, 4>& window, Player aiPlayer) const {
    int aiCount = 0;
    int oppCount = 0;
    int emptyCount = 0;
    Player opp = opponent(aiPlayer);

    for (Player p : window) {
        if (p == aiPlayer) aiCount++;
        else if (p == opp) oppCount++;
        else emptyCount++;
    }

    if (aiCount == 4) return 10000;
    if (aiCount == 3 && emptyCount == 1) return 100;
    if (aiCount == 2 && emptyCount == 2) return 10;

    if (oppCount == 3 && emptyCount == 1) return -90;
    if (oppCount == 2 && emptyCount == 2) return -8;

    return 0;
}

} // namespace Strategix::ConnectFour
