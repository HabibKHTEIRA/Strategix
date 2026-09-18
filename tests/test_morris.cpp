#include "../src/core/morris/MorrisBoard.h"
#include "../src/core/morris/MorrisGame.h"
#include "../src/core/morris/MorrisAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Morris;

void testInitialBoard() {
    std::cout << "[TEST MORRIS] Initial Board..." << std::endl;
    MorrisBoard board;
    assert(board.pieceCount(Piece::White) == 0);
    assert(board.pieceCount(Piece::Black) == 0);
    assert(board.getLegalPlacements().size() == 24);
    std::cout << "  -> Initial board OK." << std::endl;
}

void testMillDetectionAndCapture() {
    std::cout << "[TEST MORRIS] Mill detection and capture..." << std::endl;
    MorrisBoard board;
    // Outer top mill: 0, 1, 2
    board.setPiece(0, Piece::White);
    board.setPiece(1, Piece::White);
    assert(!board.isPartOfMill(0, Piece::White));

    board.setPiece(2, Piece::White);
    assert(board.isPartOfMill(0, Piece::White));
    assert(board.isPartOfMill(1, Piece::White));
    assert(board.isPartOfMill(2, Piece::White));

    // Place Black piece at 4 (non-mill) and 8, 9, 10 (mill)
    board.setPiece(4, Piece::Black);
    board.setPiece(8, Piece::Black);
    board.setPiece(9, Piece::Black);
    board.setPiece(10, Piece::Black);

    auto capturable = board.getCapturablePieces(Piece::Black);
    // Point 4 is not in a mill, so ONLY 4 should be capturable!
    assert(capturable.size() == 1);
    assert(capturable[0] == 4);
    std::cout << "  -> Mill detection and capture rules OK." << std::endl;
}

void testGameFlow() {
    std::cout << "[TEST MORRIS] Game flow placing..." << std::endl;
    MorrisGame game;
    assert(game.currentPhase(Piece::White) == GamePhase::Placing);
    bool ok = game.placePiece(0);
    assert(ok);
    (void)ok;
    assert(game.whiteUnplaced() == 8);
    assert(game.activePlayer() == Piece::Black);
    std::cout << "  -> Game flow OK." << std::endl;
}

int main() {
    std::cout << "=== Running Nine Men's Morris Unit Tests ===" << std::endl;
    testInitialBoard();
    testMillDetectionAndCapture();
    testGameFlow();
    std::cout << "=== All Nine Men's Morris Tests Passed! ===" << std::endl;
    return 0;
}
