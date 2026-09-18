#include "CheckersBoard.h"
#include <algorithm>

namespace Strategix::Checkers {

CheckersBoard::CheckersBoard() {
    reset();
}

void CheckersBoard::reset() {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            m_grid[r][c] = Piece{};
            if (isPlayableSquare(r, c)) {
                if (r < 3) {
                    m_grid[r][c] = Piece{PieceColor::Black, PieceType::Man};
                } else if (r > 4) {
                    m_grid[r][c] = Piece{PieceColor::White, PieceType::Man};
                }
            }
        }
    }
}

Piece CheckersBoard::pieceAt(int r, int c) const {
    if (!isValid(r, c)) return Piece{};
    return m_grid[r][c];
}

void CheckersBoard::setPiece(int r, int c, Piece p) {
    if (isValid(r, c)) {
        m_grid[r][c] = p;
    }
}

int CheckersBoard::whiteCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c].isWhite()) ++count;
        }
    }
    return count;
}

int CheckersBoard::blackCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c].isBlack()) ++count;
        }
    }
    return count;
}

int CheckersBoard::whiteKingsCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c].isWhite() && m_grid[r][c].isKing()) ++count;
        }
    }
    return count;
}

int CheckersBoard::blackKingsCount() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (m_grid[r][c].isBlack() && m_grid[r][c].isKing()) ++count;
        }
    }
    return count;
}

void CheckersBoard::applyMove(const Move& move) {
    Piece piece = pieceAt(move.from);
    setPiece(move.from, Piece{});

    // Remove captured pieces
    for (const auto& cap : move.captured) {
        setPiece(cap, Piece{});
    }

    // Check promotion
    if ((piece.isWhite() && move.to.row == 0) || (piece.isBlack() && move.to.row == SIZE - 1)) {
        piece.type = PieceType::King;
    }

    setPiece(move.to, piece);
}

void CheckersBoard::getJumpsFrom(Position current, Piece piece, std::vector<Position>& currentCaptured,
                                std::vector<Move>& outMoves, std::array<std::array<Piece, SIZE>, SIZE>& tempGrid) const {
    static const int dirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    bool hasFurtherJump = false;

    for (auto dir : dirs) {
        int midR = current.row + dir[0];
        int midC = current.col + dir[1];
        int landR = current.row + 2 * dir[0];
        int landC = current.col + 2 * dir[1];

        if (isValid(landR, landC) && isPlayableSquare(landR, landC)) {
            Piece midPiece = tempGrid[midR][midC];
            Piece landPiece = tempGrid[landR][landC];

            if (!midPiece.isEmpty() && midPiece.color != piece.color && landPiece.isEmpty()) {
                // Perform jump in temp grid
                tempGrid[current.row][current.col] = Piece{};
                tempGrid[midR][midC] = Piece{};
                tempGrid[landR][landC] = piece;
                currentCaptured.push_back({midR, midC});

                hasFurtherJump = true;

                // Recursive check for multi-jump
                getJumpsFrom({landR, landC}, piece, currentCaptured, outMoves, tempGrid);

                // Backtrack
                currentCaptured.pop_back();
                tempGrid[landR][landC] = Piece{};
                tempGrid[midR][midC] = midPiece;
                tempGrid[current.row][current.col] = piece;
            }
        }
    }

    if (!hasFurtherJump && !currentCaptured.empty()) {
        Move m;
        m.from = current; // Note: initial caller tracks true origin
        m.to = current;
        m.captured = currentCaptured;
        outMoves.push_back(m);
    }
}

std::vector<Move> CheckersBoard::getLegalMoves(PieceColor player) const {
    std::vector<Move> jumps;
    std::vector<Move> simpleMoves;

    auto tempGrid = m_grid;

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            Piece p = m_grid[r][c];
            if (p.color != player) continue;

            Position from{r, c};

            // 1. Check for jumps
            std::vector<Position> captured;
            std::vector<Move> pieceJumps;
            getJumpsFrom(from, p, captured, pieceJumps, tempGrid);

            for (auto& jm : pieceJumps) {
                jm.from = from; // Fix true starting position
                if ((p.isWhite() && jm.to.row == 0) || (p.isBlack() && jm.to.row == SIZE - 1)) {
                    jm.becomesKing = true;
                }
                jumps.push_back(jm);
            }

            // 2. Check for simple moves if no jumps discovered yet
            static const int whiteDirs[2][2] = {{-1, -1}, {-1, 1}};
            static const int blackDirs[2][2] = {{1, -1}, {1, 1}};
            static const int kingDirs[4][2]  = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

            const int (*dirs)[2] = p.isKing() ? kingDirs : (p.isWhite() ? whiteDirs : blackDirs);
            int dirCount = p.isKing() ? 4 : 2;

            for (int d = 0; d < dirCount; ++d) {
                int toR = r + dirs[d][0];
                int toC = c + dirs[d][1];
                if (isValid(toR, toC) && isPlayableSquare(toR, toC) && m_grid[toR][toC].isEmpty()) {
                    Move m;
                    m.from = from;
                    m.to = {toR, toC};
                    if ((p.isWhite() && toR == 0) || (p.isBlack() && toR == SIZE - 1)) {
                        m.becomesKing = true;
                    }
                    simpleMoves.push_back(m);
                }
            }
        }
    }

    // Standard rule: Mandatory captures
    if (!jumps.empty()) {
        return jumps;
    }
    return simpleMoves;
}

std::vector<Move> CheckersBoard::getLegalMovesFrom(Position from, PieceColor player) const {
    std::vector<Move> allMoves = getLegalMoves(player);
    std::vector<Move> filtered;
    for (const auto& m : allMoves) {
        if (m.from == from) {
            filtered.push_back(m);
        }
    }
    return filtered;
}

} // namespace Strategix::Checkers
