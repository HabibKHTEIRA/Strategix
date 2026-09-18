#include "../src/core/dotsandboxes/DotsAndBoxesBoard.h"
#include "../src/core/dotsandboxes/DotsAndBoxesGame.h"
#include "../src/core/dotsandboxes/DotsAndBoxesAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::DotsAndBoxes;

void testInitialBoard() {
    std::cout << "[TEST DOTS AND BOXES] Initial Board..." << std::endl;
    DotsAndBoxesBoard board;
    assert(board.score(Player::Player1) == 0);
    assert(board.score(Player::Player2) == 0);
    // 5x4 H-edges + 4x5 V-edges = 20 + 20 = 40
    assert(board.getAvailableEdges().size() == 40);
    assert(!board.isFull());
    std::cout << "  -> Initial board OK." << std::endl;
}

void testBoxCompletion() {
    std::cout << "[TEST DOTS AND BOXES] Box completion..." << std::endl;
    DotsAndBoxesGame game;

    // Draw 3 sides of box (0, 0):
    // Top: H(0, 0)
    // Left: V(0, 0)
    // Bottom: H(1, 0)
    game.playEdge(Edge{EdgeType::Horizontal, 0, 0});
    assert(game.activePlayer() == Player::Player2);

    game.playEdge(Edge{EdgeType::Vertical, 0, 0});
    assert(game.activePlayer() == Player::Player1);

    game.playEdge(Edge{EdgeType::Horizontal, 1, 0});
    assert(game.activePlayer() == Player::Player2);

    // Player 2 completes the 4th side: Right: V(0, 1)
    game.playEdge(Edge{EdgeType::Vertical, 0, 1});
    // Player 2 scores 1 point, and turn switches to Player 1 (no bonus turn)
    assert(game.player2Score() == 1);
    assert(game.activePlayer() == Player::Player1);
    std::cout << "  -> Box completion and turn pass OK." << std::endl;
}

void testAI() {
    std::cout << "[TEST DOTS AND BOXES] AI taking completed box..." << std::endl;
    DotsAndBoxesBoard board;
    board.drawEdge(Edge{EdgeType::Horizontal, 0, 0}, Player::Player1);
    board.drawEdge(Edge{EdgeType::Vertical, 0, 0}, Player::Player1);
    board.drawEdge(Edge{EdgeType::Horizontal, 1, 0}, Player::Player1);

    DotsAndBoxesAI ai(AIDifficulty::Medium);
    auto chosen = ai.chooseEdge(board, Player::Player2);
    assert(chosen.has_value());
    // AI must choose the completing edge V(0, 1)
    assert(chosen->type == EdgeType::Vertical && chosen->row == 0 && chosen->col == 1);
    (void)chosen;
    std::cout << "  -> AI capture OK." << std::endl;
}

int main() {
    std::cout << "=== Running Dots and Boxes Unit Tests ===" << std::endl;
    testInitialBoard();
    testBoxCompletion();
    testAI();
    std::cout << "=== All Dots and Boxes Tests Passed! ===" << std::endl;
    return 0;
}
