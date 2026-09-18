#include "../src/core/connectfour/ConnectFourBoard.h"
#include "../src/core/connectfour/ConnectFourGame.h"
#include "../src/core/connectfour/ConnectFourAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::ConnectFour;

void testDropAndHorizontalWin() {
    std::cout << "[TEST CONNECT FOUR] Drop and Horizontal Win..." << std::endl;
    ConnectFourBoard board;

    // Drop Red at col 0, 1, 2, 3
    for (int c = 0; c < 3; ++c) {
        board.dropPiece(c, Player::Red);
        assert(!board.checkWin(Player::Red));
    }
    board.dropPiece(3, Player::Red);
    assert(board.checkWin(Player::Red));
    assert(!board.checkWin(Player::Yellow));
    std::cout << "  -> Horizontal win OK." << std::endl;
}

void testVerticalWin() {
    std::cout << "[TEST CONNECT FOUR] Vertical Win..." << std::endl;
    ConnectFourBoard board;

    for (int i = 0; i < 3; ++i) {
        board.dropPiece(2, Player::Yellow);
        assert(!board.checkWin(Player::Yellow));
    }
    board.dropPiece(2, Player::Yellow);
    assert(board.checkWin(Player::Yellow));
    std::cout << "  -> Vertical win OK." << std::endl;
}

void testDiagonalWin() {
    std::cout << "[TEST CONNECT FOUR] Diagonal Win..." << std::endl;
    ConnectFourBoard board;

    // Build diagonal for Red at (0,0), (1,1), (2,2), (3,3)
    // col 0: Red
    board.dropPiece(0, Player::Red);
    // col 1: Yellow, Red
    board.dropPiece(1, Player::Yellow);
    board.dropPiece(1, Player::Red);
    // col 2: Yellow, Yellow, Red
    board.dropPiece(2, Player::Yellow);
    board.dropPiece(2, Player::Yellow);
    board.dropPiece(2, Player::Red);
    // col 3: Yellow, Yellow, Yellow, Red
    board.dropPiece(3, Player::Yellow);
    board.dropPiece(3, Player::Yellow);
    board.dropPiece(3, Player::Yellow);
    assert(!board.checkWin(Player::Red));

    board.dropPiece(3, Player::Red);
    assert(board.checkWin(Player::Red));
    std::cout << "  -> Diagonal win OK." << std::endl;
}

void testAIMove() {
    std::cout << "[TEST CONNECT FOUR] AI Blocking..." << std::endl;
    ConnectFourBoard board;
    board.dropPiece(0, Player::Red);
    board.dropPiece(1, Player::Red);
    board.dropPiece(2, Player::Red);

    ConnectFourAI ai(AIDifficulty::Hard);
    int chosen = ai.chooseColumn(board, Player::Yellow);
    // AI should choose col 3 to block Red's win!
    assert(chosen == 3);
    std::cout << "  -> AI blocking OK." << std::endl;
}

int main() {
    std::cout << "=== Running Connect Four Unit Tests ===" << std::endl;
    testDropAndHorizontalWin();
    testVerticalWin();
    testDiagonalWin();
    testAIMove();
    std::cout << "=== All Connect Four Tests Passed! ===" << std::endl;
    return 0;
}
