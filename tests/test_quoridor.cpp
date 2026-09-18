#include "../src/core/quoridor/QuoridorBoard.h"
#include "../src/core/quoridor/QuoridorGame.h"
#include "../src/core/quoridor/QuoridorAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Quoridor;

void testInitialBoard() {
    std::cout << "[TEST QUORIDOR] Initial positions and paths..." << std::endl;
    QuoridorBoard board;
    assert(board.playerPos(PlayerId::Player1) == Position({8, 4}));
    assert(board.playerPos(PlayerId::Player2) == Position({0, 4}));
    assert(board.shortestPathToGoal(PlayerId::Player1) == 8);
    assert(board.shortestPathToGoal(PlayerId::Player2) == 8);
    assert(board.hasPathToGoal(PlayerId::Player1));
    assert(board.hasPathToGoal(PlayerId::Player2));
    std::cout << "  -> Initial positions OK." << std::endl;
}

void testWallPlacementAndBlocking() {
    std::cout << "[TEST QUORIDOR] Wall placement and blocking..." << std::endl;
    QuoridorBoard board;

    // Place horizontal wall directly above Player 1 at (7, 4)
    Wall w{7, 4, WallOrientation::Horizontal};
    assert(board.canPlaceWall(w));
    bool placed = board.placeWall(w);
    assert(placed);
    (void)placed;
    assert(board.walls().size() == 1);

    // Player 1 at (8, 4) cannot move directly north to (7, 4)
    assert(board.isBlocked(8, 4, 7, 4));
    assert(board.isBlocked(8, 5, 7, 5));
    // But can move west (8, 3) or east (8, 5)
    assert(!board.isBlocked(8, 4, 8, 3));
    assert(!board.isBlocked(8, 4, 8, 5));

    // Path to goal is still open around the wall
    assert(board.hasPathToGoal(PlayerId::Player1));
    assert(board.shortestPathToGoal(PlayerId::Player1) > 8);
    std::cout << "  -> Wall placement and blocking OK." << std::endl;
}

void testPawnMoveAndJump() {
    std::cout << "[TEST QUORIDOR] Pawn move and jump..." << std::endl;
    QuoridorBoard board;
    board.setPlayerPos(PlayerId::Player1, {4, 4});
    board.setPlayerPos(PlayerId::Player2, {3, 4});

    // P1 at (4,4) should be able to jump over P2 at (3,4) to (2,4)
    auto moves = board.getLegalPawnMoves(PlayerId::Player1);
    bool foundJump = false;
    for (const auto& m : moves) {
        if (m == Position({2, 4})) foundJump = true;
    }
    assert(foundJump);
    (void)foundJump;
    std::cout << "  -> Pawn jump OK." << std::endl;
}

int main() {
    std::cout << "=== Running Quoridor Unit Tests ===" << std::endl;
    testInitialBoard();
    testWallPlacementAndBlocking();
    testPawnMoveAndJump();
    std::cout << "=== All Quoridor Tests Passed! ===" << std::endl;
    return 0;
}
