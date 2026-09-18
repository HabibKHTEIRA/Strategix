#include "../src/core/reversi/ReversiBoard.h"
#include "../src/core/reversi/ReversiGame.h"
#include "../src/core/reversi/ReversiAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Reversi;

void testInitialBoard() {
    std::cout << "[TEST REVERSI] Initial Board..." << std::endl;
    ReversiBoard board;
    assert(board.blackCount() == 2);
    assert(board.whiteCount() == 2);
    assert(board.emptyCount() == 60);

    // Black starts with 4 valid moves: (2,3), (3,2), (4,5), (5,4)
    auto moves = board.getLegalMoves(Piece::Black);
    assert(moves.size() == 4);
    std::cout << "  -> Initial board OK." << std::endl;
}

void testFlipping() {
    std::cout << "[TEST REVERSI] Move and Flip..." << std::endl;
    ReversiBoard board;
    // Black plays at (2, 3) bracketing White at (3, 3) against Black at (4, 3)
    int flipped = board.applyMove(2, 3, Piece::Black);
    assert(flipped == 1);
    assert(board.pieceAt(2, 3) == Piece::Black);
    assert(board.pieceAt(3, 3) == Piece::Black); // Was white, now black
    assert(board.blackCount() == 4);
    assert(board.whiteCount() == 1);
    std::cout << "  -> Flipping OK." << std::endl;
}

void testGameFlowAndAI() {
    std::cout << "[TEST REVERSI] Game Flow and AI..." << std::endl;
    ReversiGame game;
    assert(game.activePlayer() == Piece::Black);
    bool ok = game.playMove(2, 3);
    assert(ok);
    assert(game.activePlayer() == Piece::White);

    ReversiAI ai(AIDifficulty::Medium);
    auto aiMove = ai.chooseMove(game.board(), Piece::White);
    assert(aiMove.has_value());
    assert(game.board().isValidMove(aiMove->row, aiMove->col, Piece::White));
    std::cout << "  -> Game flow and AI OK." << std::endl;
}

int main() {
    std::cout << "=== Running Reversi Unit Tests ===" << std::endl;
    testInitialBoard();
    testFlipping();
    testGameFlowAndAI();
    std::cout << "=== All Reversi Tests Passed! ===" << std::endl;
    return 0;
}
