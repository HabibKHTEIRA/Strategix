#ifndef STRATEGIX_CHECKERSBOARD_H
#define STRATEGIX_CHECKERSBOARD_H

#include <array>
#include <vector>
#include <optional>

namespace Strategix::Checkers {

enum class PieceColor { None, White, Black };
enum class PieceType { Man, King };

struct Piece {
    PieceColor color = PieceColor::None;
    PieceType type = PieceType::Man;

    constexpr bool isEmpty() const { return color == PieceColor::None; }
    constexpr bool isWhite() const { return color == PieceColor::White; }
    constexpr bool isBlack() const { return color == PieceColor::Black; }
    constexpr bool isKing() const { return type == PieceType::King; }
    constexpr bool isMan() const { return type == PieceType::Man; }

    constexpr bool operator==(const Piece& other) const {
        return color == other.color && type == other.type;
    }
    constexpr bool operator!=(const Piece& other) const {
        return !(*this == other);
    }
};

struct Position {
    int row = 0;
    int col = 0;

    constexpr bool operator==(const Position& o) const { return row == o.row && col == o.col; }
    constexpr bool operator!=(const Position& o) const { return !(*this == o); }
};

struct Move {
    Position from;
    Position to;
    std::vector<Position> captured; // positions of captured pieces in sequence
    bool becomesKing = false;
};

class CheckersBoard {
public:
    static constexpr int SIZE = 8;

    CheckersBoard();

    void reset();

    static bool isPlayableSquare(int r, int c) {
        return isValid(r, c) && ((r + c) % 2 != 0);
    }

    static bool isValid(int r, int c) {
        return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
    }

    Piece pieceAt(int r, int c) const;
    Piece pieceAt(Position p) const { return pieceAt(p.row, p.col); }

    void setPiece(int r, int c, Piece p);
    void setPiece(Position pos, Piece p) { setPiece(pos.row, pos.col, p); }

    int whiteCount() const;
    int blackCount() const;
    int whiteKingsCount() const;
    int blackKingsCount() const;

    // Apply and undo move directly on board
    void applyMove(const Move& move);

    // Generates all legal moves for the given player.
    // If captures are available, only capture moves are returned (standard mandatory capture rule).
    std::vector<Move> getLegalMoves(PieceColor player) const;

    // Get legal moves starting specifically from a given position (for UI click & move selection)
    std::vector<Move> getLegalMovesFrom(Position from, PieceColor player) const;

private:
    void getJumpsFrom(Position current, Piece piece, std::vector<Position>& currentCaptured,
                     std::vector<Move>& outMoves, std::array<std::array<Piece, SIZE>, SIZE>& tempGrid) const;

    std::array<std::array<Piece, SIZE>, SIZE> m_grid;
};

} // namespace Strategix::Checkers

#endif // STRATEGIX_CHECKERSBOARD_H
