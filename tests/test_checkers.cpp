#include "../src/core/checkers/CheckersBoard.h"
#include "../src/core/checkers/CheckersGame.h"
#include "../src/core/checkers/CheckersAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Checkers;

void testInitialBoard() {
    std::cout << "[TEST CHECKERS] Board initialization..." << std::endl;
    CheckersBoard board;
    assert(board.whiteCount() == 12);
    assert(board.blackCount() == 12);
    assert(board.whiteKingsCount() == 0);
    assert(board.blackKingsCount() == 0);

    // Check White pieces are at bottom (rows 5, 6, 7)
    for (int c = 0; c < 8; ++c) {
        if ((5 + c) % 2 != 0) assert(board.pieceAt(5, c).isWhite());
        if ((0 + c) % 2 != 0) assert(board.pieceAt(0, c).isBlack());
    }
    std::cout << "  -> Initial board OK." << std::endl;
}

void testPawnMoveAndCapture() {
    std::cout << "[TEST CHECKERS] Pawn moves and captures..." << std::endl;
    CheckersBoard board;
    // Clear board
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            board.setPiece(r, c, Piece{});
        }
    }

    // White pawn at (5, 2), Black pawn at (4, 3)
    board.setPiece(5, 2, Piece{PieceColor::White, PieceType::Man});
    board.setPiece(4, 3, Piece{PieceColor::Black, PieceType::Man});

    // White should have a jump available: from (5, 2) over (4, 3) to (3, 4)
    auto moves = board.getLegalMoves(PieceColor::White);
    assert(moves.size() == 1);
    assert(moves[0].from == Position({5, 2}));
    assert(moves[0].to == Position({3, 4}));
    assert(moves[0].captured.size() == 1);
    assert(moves[0].captured[0] == Position({4, 3}));

    // Apply move
    board.applyMove(moves[0]);
    assert(board.pieceAt(5, 2).isEmpty());
    assert(board.pieceAt(4, 3).isEmpty()); // Captured!
    assert(board.pieceAt(3, 4).isWhite());

    std::cout << "  -> Pawn capture OK." << std::endl;
}

void testKingPromotion() {
    std::cout << "[TEST CHECKERS] King promotion..." << std::endl;
    CheckersBoard board;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            board.setPiece(r, c, Piece{});
        }
    }

    // White pawn at (1, 2), moves to (0, 1) -> should become King!
    board.setPiece(1, 2, Piece{PieceColor::White, PieceType::Man});
    auto moves = board.getLegalMovesFrom(Position({1, 2}), PieceColor::White);
    assert(!moves.empty());

    Move m = moves[0];
    assert(m.to.row == 0);
    assert(m.becomesKing);

    board.applyMove(m);
    assert(board.pieceAt(m.to).isKing());
    assert(board.pieceAt(m.to).isWhite());

    std::cout << "  -> King promotion OK." << std::endl;
}

void testCheckersGameAndAI() {
    std::cout << "[TEST CHECKERS] Game loop and AI..." << std::endl;
    CheckersGame game;
    game.startNewGame(GameMode::PlayerVsAI, AIDifficulty::Medium);
    assert(game.activePlayer() == PieceColor::White);
    assert(game.status() == GameStatus::InProgress);

    auto moves = game.getLegalMoves();
    assert(!moves.empty());

    // Play first move
    game.makeMove(moves[0]);
    assert(game.activePlayer() == PieceColor::Black);

    // AI plays Black
    CheckersAI ai(AIDifficulty::Medium);
    Move aiMove = ai.chooseMove(game.board(), PieceColor::Black);
    assert(aiMove.from.row >= 0 && aiMove.to.row >= 0);
    game.makeMove(aiMove);

    assert(game.activePlayer() == PieceColor::White);

    // Test Undo
    assert(game.canUndo());
    assert(game.undo());

    std::cout << "  -> Game loop and AI OK." << std::endl;
}

int main() {
    std::cout << "=== Running Checkers Unit Tests ===" << std::endl;
    testInitialBoard();
    testPawnMoveAndCapture();
    testKingPromotion();
    testCheckersGameAndAI();
    std::cout << "=== ALL CHECKERS TESTS PASSED! ===" << std::endl;
    return 0;
}
