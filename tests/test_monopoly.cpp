#include "../src/core/monopoly/MonopolyBoard.h"
#include "../src/core/monopoly/MonopolyGame.h"
#include "../src/core/monopoly/MonopolyAI.h"

#include <cassert>
#include <iostream>

using namespace Strategix::Monopoly;

void testInitialBoard() {
    std::cout << "[TEST MONOPOLY] Testing initial 40-space board..." << std::endl;
    MonopolyBoard board;
    assert(board.space(0).type == SpaceType::Go);
    assert(board.space(1).group == ColorGroup::Brown);
    assert(board.space(1).price == 60);
    assert(board.space(5).type == SpaceType::Railroad);
    assert(board.space(10).type == SpaceType::Jail);
    assert(board.space(12).type == SpaceType::Utility);
    assert(board.space(20).type == SpaceType::FreeParking);
    assert(board.space(30).type == SpaceType::GoToJail);
    assert(board.space(39).group == ColorGroup::DarkBlue);
    assert(board.space(39).price == 400);
    assert(board.bankHousesAvailable() == 32);
    assert(board.bankHotelsAvailable() == 12);
    std::cout << "  -> Initial 40-space board OK." << std::endl;
}

void testBuyingAndRentAndEvenBuilding() {
    std::cout << "[TEST MONOPOLY] Testing buying, double group rent, and even building..." << std::endl;
    MonopolyGame game;
    game.startNewGame(GameMode::TwoPlayers, AIDifficulty::Medium);
    game.dismissStartModal();

    // Roll (1, 2) to land on space 3 (Rue Lecourbe, M60)
    game.rollDice(1, 2);
    assert(game.activePlayer().position == 3);
    assert(game.phase() == TurnPhase::BuyOrAuction);

    bool bought = game.buyCurrentProperty();
    assert(bought);
    (void)bought;
    assert(game.board().space(3).ownerId == 0);
    assert(game.player(0).money == 1440);

    // Base rent on Lecourbe without Belleville is 4
    assert(game.board().calculateRent(3, 7) == 4);

    // Give Belleville (1) to Player 0 as well -> full Brown group!
    game.boardMut().spaceMut(1).ownerId = 0;
    assert(game.board().ownsFullColorGroup(0, ColorGroup::Brown));

    // Unimproved rent is now doubled (8)!
    assert(game.board().calculateRent(3, 7) == 8);

    // Build 1 house on Belleville (1)
    assert(game.board().canBuildOn(0, 1, game.player(0).money));
    bool built1 = game.buildHouseOrHotel(0, 1);
    assert(built1);
    (void)built1;

    // Even building rule: cannot build a 2nd house on Belleville before building 1 on Lecourbe!
    assert(!game.board().canBuildOn(0, 1, game.player(0).money));
    assert(game.board().canBuildOn(0, 3, game.player(0).money));

    bool built2 = game.buildHouseOrHotel(0, 3);
    assert(built2);
    (void)built2;
    assert(game.board().canBuildOn(0, 1, game.player(0).money));
    std::cout << "  -> Buying, double group rent, and even building OK." << std::endl;
}

void testAuctionAndMortgageAndAI() {
    std::cout << "[TEST MONOPOLY] Testing auctions, mortgages, and AI decisions..." << std::endl;
    MonopolyGame game;
    MonopolyAI ai;
    game.startNewGame(GameMode::PlayerVsAI, AIDifficulty::Hard);
    game.dismissStartModal();

    // Roll to space 6 (Rue de Vaugirard, M100)
    game.rollDice(2, 4);
    assert(game.activePlayer().position == 6);
    assert(game.phase() == TurnPhase::BuyOrAuction);

    // Start auction
    game.startAuctionForCurrentProperty();
    assert(game.phase() == TurnPhase::Auction);
    assert(game.auctionState().activeBidderId == 0);

    // Player 0 bids 40
    bool bidOk = game.placeAuctionBid(0, 40);
    assert(bidOk);
    (void)bidOk;
    assert(game.auctionState().activeBidderId == 1);

    // AI (Player 1) chooses a bid
    int aiBid = ai.chooseAuctionBid(game, 1);
    assert(aiBid > 40);
    game.placeAuctionBid(1, aiBid);

    // Player 0 folds -> AI wins the auction!
    game.foldAuction(0);
    assert(game.phase() == TurnPhase::WaitingForEndTurn);
    assert(game.board().space(6).ownerId == 1);
    assert(game.player(1).money == 1500 - aiBid);

    // Test mortgaging and unmortgaging for AI
    bool mortOk = game.mortgageProperty(1, 6);
    assert(mortOk);
    (void)mortOk;
    assert(game.board().space(6).mortgaged);
    assert(game.board().calculateRent(6, 7) == 0);

    bool unmortOk = game.unmortgageProperty(1, 6);
    assert(unmortOk);
    (void)unmortOk;
    assert(!game.board().space(6).mortgaged);
    std::cout << "  -> Auctions, mortgages, and AI decisions OK." << std::endl;
}

void testThreeDoublesJail() {
    std::cout << "[TEST MONOPOLY] Testing 3 consecutive doubles send to Jail..." << std::endl;
    MonopolyGame game;
    game.startNewGame(GameMode::TwoPlayers, AIDifficulty::Easy);
    game.dismissStartModal();

    // 1st double: (1, 1) -> space 2 (Community Chest) or (3, 3) -> space 6
    game.rollDice(3, 3);
    game.buyCurrentProperty();
    assert(game.canRollAgain());

    // 2nd double: (4, 4) -> space 14
    game.rollDice(4, 4);
    game.buyCurrentProperty();
    assert(game.canRollAgain());

    // 3rd double: (2, 2) -> sent straight to Jail (space 10)!
    game.rollDice(2, 2);
    assert(game.activePlayer().inJail);
    assert(game.activePlayer().position == 10);
    assert(!game.canRollAgain());
    std::cout << "  -> 3 consecutive doubles to Jail OK." << std::endl;
}

int main() {
    std::cout << "=== Running Monopoly Unit Tests ===" << std::endl;
    testInitialBoard();
    testBuyingAndRentAndEvenBuilding();
    testAuctionAndMortgageAndAI();
    testThreeDoublesJail();
    std::cout << "=== All Monopoly Tests Passed! ===" << std::endl;
    return 0;
}
