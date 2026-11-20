#ifndef STRATEGIX_MONOPOLYGAME_H
#define STRATEGIX_MONOPOLYGAME_H

#include "MonopolyBoard.h"
#include <string>
#include <vector>
#include <optional>
#include <random>

namespace Strategix::Monopoly {

enum class TokenType {
    Racecar = 0,
    Battleship = 1,
    TopHat = 2,
    Dog = 3,
    Boot = 4,
    Cat = 5,
    Thimble = 6,
    Wheelbarrow = 7
};

enum class AIDifficulty {
    Easy = 0,     // Agent immobilier
    Medium = 1,   // Entrepreneur
    Hard = 2      // Magnat de l'immobilier
};

enum class GameMode {
    TwoPlayers,
    PlayerVsAI
};

enum class GameStatus {
    InProgress,
    Finished
};

enum class TurnPhase {
    WaitingForStartModal,
    WaitingForRoll,
    BuyOrAuction,
    Auction,
    RentPaymentModal,
    CardModal,
    DebtResolution,
    WaitingForEndTurn,
    GameOver
};

struct PlayerSetup {
    std::string customName;
    TokenType token = TokenType::Racecar;
    bool isAI = false;
    AIDifficulty difficulty = AIDifficulty::Easy;
};

struct Player {
    int id = 0;
    std::string customName;
    TokenType token = TokenType::Racecar;
    bool isAI = false;
    AIDifficulty difficulty = AIDifficulty::Easy;
    int money = 1500;
    int position = 0;
    bool inJail = false;
    int jailTurns = 0;
    int doublesCount = 0;
    bool hasChanceJailCard = false;
    bool hasChestJailCard = false;
    bool bankrupt = false;

    int jailFreeCards() const {
        return (hasChanceJailCard ? 1 : 0) + (hasChestJailCard ? 1 : 0);
    }
};

enum class CardActionType {
    CollectMoney,
    PayMoney,
    MoveTo,
    MoveRelative,
    MoveToNearestRailroad,
    MoveToNearestUtility,
    GoToJail,
    GetOutOfJailFree,
    StreetRepairs,
    PayEachPlayer,
    CollectFromEachPlayer
};

struct Card {
    int id = 0;
    bool isChance = true;
    std::string textKey;
    CardActionType action = CardActionType::CollectMoney;
    int amount = 0;
    int targetSpace = 0;
    int perHouse = 0;
    int perHotel = 0;
};

struct AuctionState {
    int spaceIndex = -1;
    int currentBid = 0;
    int highestBidderId = -1;
    int activeBidderId = 0;
    int inputBid = 1;
    std::vector<bool> folded;
};

struct RentNotification {
    int fromPlayerId = -1;
    int toPlayerId = -1;
    int spaceIndex = -1;
    int amount = 0;
};

struct TradeOffer {
    int fromPlayerId = 0;
    int toPlayerId = 1;
    std::vector<int> offeredProperties;
    int offeredCash = 0;
    int offeredJailCards = 0;
    std::vector<int> requestedProperties;
    int requestedCash = 0;
    int requestedJailCards = 0;
};

struct HistoryEntry {
    int elapsedSeconds = 0;
    std::string key;
    int player1Id = -1;
    int player2Id = -1;
    int spaceIndex = -1;
    int amount = 0;
};

struct MoneyTransferEvent {
    int fromPlayerId = -1; // -1 = Bank
    int toPlayerId = -1;   // -1 = Bank
    int amount = 0;
    int spaceIndex = -1;
};

struct GameSnapshot {
    MonopolyBoard board;
    std::vector<Player> players;
    int activePlayerIndex = 0;
    TurnPhase phase = TurnPhase::WaitingForRoll;
    GameStatus status = GameStatus::InProgress;
    int die1 = 1;
    int die2 = 1;
    bool canRollAgain = false;
    int pendingDebtAmount = 0;
    int pendingCreditorId = -1;
    std::vector<HistoryEntry> historyLog;
};

class MonopolyGame {
public:
    MonopolyGame();

    void startNewGame(GameMode mode = GameMode::PlayerVsAI, AIDifficulty diff = AIDifficulty::Medium);
    void startCustomGame(const std::vector<PlayerSetup>& setups);

    const MonopolyBoard& board() const { return m_board; }
    MonopolyBoard& boardMut() { return m_board; }

    const std::vector<Player>& players() const { return m_players; }
    const Player& player(int id) const { return m_players[id]; }
    Player& playerMut(int id) { return m_players[id]; }
    int numPlayers() const { return static_cast<int>(m_players.size()); }

    int activePlayerIndex() const { return m_activePlayerIndex; }
    const Player& activePlayer() const { return m_players[m_activePlayerIndex]; }
    Player& activePlayerMut() { return m_players[m_activePlayerIndex]; }

    TurnPhase phase() const { return m_phase; }
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }
    AIDifficulty difficulty() const { return m_difficulty; }
    int winnerId() const { return m_winnerId; }

    bool isAITurn() const;
    int activeActorId() const; // During auction, returns activeBidderId; otherwise activePlayerIndex

    int die1() const { return m_die1; }
    int die2() const { return m_die2; }
    int diceSum() const { return m_die1 + m_die2; }
    bool isDoubleRoll() const { return m_die1 == m_die2; }
    bool canRollAgain() const { return m_canRollAgain; }

    int elapsedSeconds() const { return m_elapsedSeconds; }
    void setElapsedSeconds(int sec) { m_elapsedSeconds = sec; }

    // Turn progression
    void dismissStartModal();
    std::pair<int, int> rollDice(int forcedDie1 = 0, int forcedDie2 = 0);
    bool payJailFine();
    bool useJailFreeCard();

    // Landing & Modals
    bool buyCurrentProperty();
    void startAuctionForCurrentProperty();
    bool placeAuctionBid(int playerId, int bidAmount);
    bool foldAuction(int playerId);
    const AuctionState& auctionState() const { return m_auction; }
    void setAuctionInputBid(int bid) { m_auction.inputBid = bid; }

    const RentNotification& rentNotification() const { return m_rentNotif; }
    void dismissRentModal();

    const Card& drawnCard() const { return m_drawnCard; }
    void dismissCardModal();

    // Building, Selling, Mortgaging, Trading
    bool buildHouseOrHotel(int playerId, int spaceIndex);
    bool sellHouseOrHotel(int playerId, int spaceIndex);
    bool mortgageProperty(int playerId, int spaceIndex);
    bool unmortgageProperty(int playerId, int spaceIndex);
    bool executeTrade(const TradeOffer& offer);

    // Debt & Bankruptcy
    int pendingDebtAmount() const { return m_pendingDebtAmount; }
    int pendingCreditorId() const { return m_pendingCreditorId; }
    int playerNetWorth(int playerId) const;
    int playerLiquidatableValue(int playerId) const;
    bool settlePendingDebt();
    void declareBankruptcy(int playerId);

    void endTurn();

    // Events for UI animations
    const std::vector<int>& lastMovePath() const { return m_lastMovePath; }
    void clearLastMovePath() { m_lastMovePath.clear(); }
    bool lastMoveWasJailSend() const { return m_lastMoveWasJailSend; }

    const std::vector<MoneyTransferEvent>& recentTransfers() const { return m_recentTransfers; }
    void clearRecentTransfers() { m_recentTransfers.clear(); }

    const std::vector<HistoryEntry>& historyLog() const { return m_historyLog; }

    bool canUndo() const { return !m_undoStack.empty(); }
    void undo();

private:
    void initDecks();
    void recordSnapshot();
    void addHistory(const std::string& key, int p1 = -1, int p2 = -1, int spaceIdx = -1, int amount = 0);
    void recordTransfer(int fromId, int toId, int amount, int spaceIdx = -1);

    void moveActivePlayerBy(int steps);
    void moveActivePlayerTo(int targetSpace, bool collectGoIfPassed = true);
    void sendActivePlayerToJail();
    void resolveLanding(int spaceIndex, int rentMultiplier = 1);
    void drawCard(bool isChance);
    void applyDrawnCard();
    void requirePayment(int payerId, int creditorId, int amount, int spaceIdx = -1);
    void advanceAuctionBidder();
    void finalizeAuction();
    void checkVictory();

    MonopolyBoard m_board;
    std::vector<Player> m_players;
    int m_activePlayerIndex = 0;
    TurnPhase m_phase = TurnPhase::WaitingForStartModal;
    GameStatus m_status = GameStatus::InProgress;
    GameMode m_mode = GameMode::PlayerVsAI;
    AIDifficulty m_difficulty = AIDifficulty::Medium;
    int m_winnerId = -1;

    int m_die1 = 1;
    int m_die2 = 1;
    bool m_canRollAgain = false;
    int m_pendingRentMultiplier = 1;

    int m_pendingDebtAmount = 0;
    int m_pendingCreditorId = -1; // -1 = Bank

    AuctionState m_auction;
    RentNotification m_rentNotif;
    Card m_drawnCard;

    std::vector<Card> m_chanceDeck;
    std::vector<Card> m_chestDeck;
    size_t m_chanceIndex = 0;
    size_t m_chestIndex = 0;

    int m_elapsedSeconds = 0;
    std::vector<HistoryEntry> m_historyLog;
    std::vector<int> m_lastMovePath;
    bool m_lastMoveWasJailSend = false;
    std::vector<MoneyTransferEvent> m_recentTransfers;
    std::vector<GameSnapshot> m_undoStack;

    std::mt19937 m_rng;
};

} // namespace Strategix::Monopoly

#endif // STRATEGIX_MONOPOLYGAME_H
