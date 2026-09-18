#include "../src/core/go/GoBoard.h"
#include "../src/core/go/GoGame.h"
#include "../src/core/go/GoAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Go;

void testGoLibertiesAndCapture() {
    std::cout << "[TEST GO] Liberties and Capture..." << std::endl;
    GoBoard board(9);

    // Place a White stone at (4, 4)
    board.setStone(4, 4, StoneColor::White);
    Group g = board.getGroup({4, 4});
    assert(g.liberties.size() == 4);

    // Surround it with 3 Black stones
    board.setStone(3, 4, StoneColor::Black);
    board.setStone(5, 4, StoneColor::Black);
    board.setStone(4, 3, StoneColor::Black);
    g = board.getGroup({4, 4});
    assert(g.liberties.size() == 1); // 1 liberty left at (4, 5)

    // Play Black stone at (4, 5) -> captures White stone!
    auto captured = board.playMove({4, 5}, StoneColor::Black);
    assert(captured.size() == 1);
    assert(captured[0] == Point({4, 4}));
    assert(board.stoneAt(4, 4) == StoneColor::None); // Empty again!

    std::cout << "  -> Capture OK." << std::endl;
}

void testGoSuicideRule() {
    std::cout << "[TEST GO] Suicide rule..." << std::endl;
    GoBoard board(9);
    // Surround (0, 0) with Black stones at (0, 1) and (1, 0)
    board.setStone(0, 1, StoneColor::Black);
    board.setStone(1, 0, StoneColor::Black);

    // White playing at (0, 0) would have 0 liberties and captures no stones -> Suicide!
    assert(!board.isLegalMove({0, 0}, StoneColor::White, std::nullopt));

    // But Black playing at (0, 0) is legal because it connects to Black stones with liberties
    assert(board.isLegalMove({0, 0}, StoneColor::Black, std::nullopt));

    std::cout << "  -> Suicide rule OK." << std::endl;
}

void testGoKoRule() {
    std::cout << "[TEST GO] Ko rule..." << std::endl;
    GoGame game(9);

    // Setup classic Ko pattern:
    // Black at (1, 2), (2, 1), (3, 2)
    // White at (2, 3), (1, 4), (3, 4)
    // and White stone at (2, 2)
    game.playMove({2, 1}); // Black
    game.playMove({1, 3}); // White
    game.playMove({1, 2}); // Black
    game.playMove({3, 3}); // White
    game.playMove({3, 2}); // Black
    game.playMove({2, 4}); // White
    game.playMove({8, 8}); // Black plays elsewhere
    game.playMove({2, 2}); // White: now at (2, 2) with 1 liberty at (2, 3)

    // Black plays at (2, 3), capturing White at (2, 2)
    // This creates Ko point at (2, 2)
    assert(game.isLegalMove({2, 3}));
    game.playMove({2, 3});

    assert(game.koPoint().has_value());
    assert(*game.koPoint() == Point({2, 2}));

    // White cannot immediately recapture at (2, 2)!
    assert(!game.isLegalMove({2, 2}));

    std::cout << "  -> Ko rule OK." << std::endl;
}

void testGoPassingAndAI() {
    std::cout << "[TEST GO] Passing and AI..." << std::endl;
    GoGame game(9);
    game.startNewGame(GameMode::PlayerVsAI, AIDifficulty::Medium);
    assert(game.activePlayer() == StoneColor::Black);

    // Black plays a move
    assert(game.playMove({4, 4}));
    assert(game.activePlayer() == StoneColor::White);

    // AI plays White
    GoAI ai(AIDifficulty::Medium);
    auto moveOpt = ai.chooseMove(game.board(), StoneColor::White, game.koPoint());
    assert(moveOpt.has_value());
    game.playMove(*moveOpt);
    assert(game.activePlayer() == StoneColor::Black);

    // 2 Consecutive passes ends game
    game.passTurn();
    game.passTurn();
    assert(game.status() == GameStatus::Finished);

    std::cout << "  -> Passing and AI OK." << std::endl;
}

int main() {
    std::cout << "=== Running Go Unit Tests ===" << std::endl;
    testGoLibertiesAndCapture();
    testGoSuicideRule();
    testGoKoRule();
    testGoPassingAndAI();
    std::cout << "=== ALL GO TESTS PASSED! ===" << std::endl;
    return 0;
}
