#include "MonopolyGame.h"
#include <algorithm>
#include <chrono>

namespace Strategix::Monopoly {

MonopolyGame::MonopolyGame() {
    auto seed = static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
    initDecks();
    startNewGame(GameMode::PlayerVsAI, AIDifficulty::Medium);
}

void MonopolyGame::initDecks() {
    m_chanceDeck = {
        {0,  true, "mono_chance_0",  CardActionType::MoveTo, 0, 0, 0, 0},                    // Avancez jusqu'à la case Départ (200 M)
        {1,  true, "mono_chance_1",  CardActionType::MoveTo, 0, 39, 0, 0},                   // Rendez-vous Rue de la Paix
        {2,  true, "mono_chance_2",  CardActionType::MoveTo, 0, 24, 0, 0},                   // Rendez-vous Avenue Henri-Martin
        {3,  true, "mono_chance_3",  CardActionType::MoveTo, 0, 11, 0, 0},                   // Avancez au Boulevard de la Villette
        {4,  true, "mono_chance_4",  CardActionType::MoveTo, 0, 15, 0, 0},                   // Rendez-vous à la Gare de Lyon
        {5,  true, "mono_chance_5",  CardActionType::MoveToNearestRailroad, 0, 0, 0, 0},     // Avancez jusqu'à la gare la plus proche
        {6,  true, "mono_chance_6",  CardActionType::MoveToNearestUtility, 0, 0, 0, 0},      // Avancez jusqu'à la compagnie la plus proche
        {7,  true, "mono_chance_7",  CardActionType::MoveRelative, -3, 0, 0, 0},             // Reculez de trois cases
        {8,  true, "mono_chance_8",  CardActionType::GoToJail, 0, 10, 0, 0},                 // Allez en prison
        {9,  true, "mono_chance_9",  CardActionType::GetOutOfJailFree, 0, 0, 0, 0},          // Vous êtes libéré de prison
        {10, true, "mono_chance_10", CardActionType::StreetRepairs, 0, 0, 25, 100},          // Travaux sur vos propriétés (25M/maison, 100M/hôtel)
        {11, true, "mono_chance_11", CardActionType::PayMoney, 15, 0, 0, 0},                 // Amende pour excès de vitesse sur le périphérique : payez 15 M
        {12, true, "mono_chance_12", CardActionType::CollectMoney, 100, 0, 0, 0},            // Vous aidez à ramasser les déchets dans Paris : recevez 100 M
        {13, true, "mono_chance_13", CardActionType::CollectMoney, 50, 0, 0, 0},             // La Banque vous verse un dividende de 50 M
        {14, true, "mono_chance_14", CardActionType::CollectMoney, 150, 0, 0, 0},            // Votre immeuble et votre prêt rapportent : recevez 150 M
        {15, true, "mono_chance_15", CardActionType::PayEachPlayer, 50, 0, 0, 0}             // Élu président du conseil : versez 50 M à chaque joueur
    };

    m_chestDeck = {
        {0,  false, "mono_chest_0",  CardActionType::MoveTo, 0, 0, 0, 0},                    // Avancez jusqu'à la case Départ (200 M)
        {1,  false, "mono_chest_1",  CardActionType::CollectMoney, 200, 0, 0, 0},            // Erreur de la Banque en votre faveur : recevez 200 M
        {2,  false, "mono_chest_2",  CardActionType::PayMoney, 50, 0, 0, 0},                 // Visitez le médecin : payez 50 M
        {3,  false, "mono_chest_3",  CardActionType::CollectMoney, 50, 0, 0, 0},             // La vente de votre stock vous rapporte 50 M
        {4,  false, "mono_chest_4",  CardActionType::GetOutOfJailFree, 0, 0, 0, 0},          // Vous êtes libéré de prison
        {5,  false, "mono_chest_5",  CardActionType::GoToJail, 0, 10, 0, 0},                 // Allez en prison
        {6,  false, "mono_chest_6",  CardActionType::CollectFromEachPlayer, 10, 0, 0, 0},    // C'est votre anniversaire : chaque joueur vous donne 10 M
        {7,  false, "mono_chest_7",  CardActionType::CollectMoney, 100, 0, 0, 0},            // Votre assurance vie vous rapporte 100 M
        {8,  false, "mono_chest_8",  CardActionType::PayMoney, 100, 0, 0, 0},                // Frais d'hospitalisation : payez 100 M
        {9,  false, "mono_chest_9",  CardActionType::PayMoney, 50, 0, 0, 0},                 // Frais de scolarité : payez 50 M
        {10, false, "mono_chest_10", CardActionType::CollectMoney, 25, 0, 0, 0},             // Recevez 25 M d'honoraires de conseil
        {11, false, "mono_chest_11", CardActionType::StreetRepairs, 0, 0, 40, 115},          // Réparations de voirie (40M/maison, 115M/hôtel)
        {12, false, "mono_chest_12", CardActionType::CollectMoney, 10, 0, 0, 0},             // Deuxième prix de beauté : recevez 10 M
        {13, false, "mono_chest_13", CardActionType::CollectMoney, 100, 0, 0, 0},            // Vous héritez de 100 M
        {14, false, "mono_chest_14", CardActionType::CollectMoney, 150, 0, 0, 0},            // Les impôts vous remboursent : recevez 150 M
        {15, false, "mono_chest_15", CardActionType::MoveTo, 0, 1, 0, 0}                     // Retournez à Belleville
    };
}

void MonopolyGame::startNewGame(GameMode mode, AIDifficulty diff) {
    m_mode = mode;
    m_difficulty = diff;

    std::vector<PlayerSetup> setups(2);
    setups[0].customName = "";
    setups[0].token = TokenType::Racecar;
    setups[0].isAI = false;
    setups[0].difficulty = diff;

    setups[1].customName = "";
    setups[1].token = TokenType::Battleship;
    setups[1].isAI = (mode == GameMode::PlayerVsAI);
    setups[1].difficulty = diff;

    startCustomGame(setups);
}

void MonopolyGame::startCustomGame(const std::vector<PlayerSetup>& setups) {
    m_board.reset();
    m_players.clear();
    m_historyLog.clear();
    m_lastMovePath.clear();
    m_lastMoveWasJailSend = false;
    m_recentTransfers.clear();
    m_undoStack.clear();

    int count = std::clamp(static_cast<int>(setups.size()), 2, 4);
    bool anyAI = false;
    for (int i = 0; i < count; ++i) {
        Player p;
        p.id = i;
        p.customName = setups[i].customName;
        p.token = setups[i].token;
        p.isAI = setups[i].isAI;
        p.difficulty = setups[i].difficulty;
        p.money = 1500;
        p.position = 0;
        if (p.isAI) {
            anyAI = true;
            m_difficulty = p.difficulty;
        }
        m_players.push_back(p);
    }
    m_mode = anyAI ? GameMode::PlayerVsAI : GameMode::TwoPlayers;

    std::shuffle(m_chanceDeck.begin(), m_chanceDeck.end(), m_rng);
    std::shuffle(m_chestDeck.begin(), m_chestDeck.end(), m_rng);
    m_chanceIndex = 0;
    m_chestIndex = 0;

    m_activePlayerIndex = 0;
    m_phase = TurnPhase::WaitingForStartModal;
    m_status = GameStatus::InProgress;
    m_winnerId = -1;
    m_die1 = 1;
    m_die2 = 1;
    m_canRollAgain = false;
    m_pendingDebtAmount = 0;
    m_pendingCreditorId = -1;
    m_elapsedSeconds = 0;

    addHistory("mono_log_welcome");
}

bool MonopolyGame::isAITurn() const {
    if (m_status != GameStatus::InProgress) return false;
    int actor = activeActorId();
    if (actor < 0 || actor >= static_cast<int>(m_players.size())) return false;
    return m_players[actor].isAI;
}

int MonopolyGame::activeActorId() const {
    if (m_phase == TurnPhase::Auction && m_auction.activeBidderId >= 0) {
        return m_auction.activeBidderId;
    }
    return m_activePlayerIndex;
}

void MonopolyGame::recordSnapshot() {
    GameSnapshot snap;
    snap.board = m_board;
    snap.players = m_players;
    snap.activePlayerIndex = m_activePlayerIndex;
    snap.phase = m_phase;
    snap.status = m_status;
    snap.die1 = m_die1;
    snap.die2 = m_die2;
    snap.canRollAgain = m_canRollAgain;
    snap.pendingDebtAmount = m_pendingDebtAmount;
    snap.pendingCreditorId = m_pendingCreditorId;
    snap.historyLog = m_historyLog;
    m_undoStack.push_back(std::move(snap));
    if (m_undoStack.size() > 30) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

void MonopolyGame::undo() {
    if (m_undoStack.empty()) return;
    GameSnapshot snap = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    m_board = std::move(snap.board);
    m_players = std::move(snap.players);
    m_activePlayerIndex = snap.activePlayerIndex;
    m_phase = snap.phase;
    m_status = snap.status;
    m_die1 = snap.die1;
    m_die2 = snap.die2;
    m_canRollAgain = snap.canRollAgain;
    m_pendingDebtAmount = snap.pendingDebtAmount;
    m_pendingCreditorId = snap.pendingCreditorId;
    m_historyLog = std::move(snap.historyLog);
    m_lastMovePath.clear();
    m_recentTransfers.clear();
}

void MonopolyGame::addHistory(const std::string& key, int p1, int p2, int spaceIdx, int amount) {
    HistoryEntry entry;
    entry.elapsedSeconds = m_elapsedSeconds;
    entry.key = key;
    entry.player1Id = p1;
    entry.player2Id = p2;
    entry.spaceIndex = spaceIdx;
    entry.amount = amount;
    m_historyLog.insert(m_historyLog.begin(), entry);
    if (m_historyLog.size() > 100) {
        m_historyLog.pop_back();
    }
}

void MonopolyGame::recordTransfer(int fromId, int toId, int amount, int spaceIdx) {
    if (amount <= 0) return;
    m_recentTransfers.push_back({fromId, toId, amount, spaceIdx});
}

void MonopolyGame::dismissStartModal() {
    if (m_phase == TurnPhase::WaitingForStartModal) {
        m_phase = TurnPhase::WaitingForRoll;
    }
}

bool MonopolyGame::payJailFine() {
    Player& p = activePlayerMut();
    if (!p.inJail || m_phase != TurnPhase::WaitingForRoll || p.money < 50) return false;
    recordSnapshot();
    p.money -= 50;
    p.inJail = false;
    p.jailTurns = 0;
    recordTransfer(p.id, -1, 50, 10);
    addHistory("mono_log_paid_jail", p.id, -1, 10, 50);
    return true;
}

bool MonopolyGame::useJailFreeCard() {
    Player& p = activePlayerMut();
    if (!p.inJail || m_phase != TurnPhase::WaitingForRoll || p.jailFreeCards() <= 0) return false;
    recordSnapshot();
    if (p.hasChanceJailCard) {
        p.hasChanceJailCard = false;
    } else if (p.hasChestJailCard) {
        p.hasChestJailCard = false;
    }
    p.inJail = false;
    p.jailTurns = 0;
    addHistory("mono_log_used_jail_card", p.id);
    return true;
}

std::pair<int, int> MonopolyGame::rollDice(int forcedDie1, int forcedDie2) {
    if (m_phase != TurnPhase::WaitingForRoll && !(m_phase == TurnPhase::WaitingForEndTurn && m_canRollAgain)) {
        return {m_die1, m_die2};
    }

    if (!activePlayer().isAI) {
        recordSnapshot();
    }

    m_lastMovePath.clear();
    m_lastMoveWasJailSend = false;
    m_recentTransfers.clear();
    m_pendingRentMultiplier = 1;

    std::uniform_int_distribution<int> dist(1, 6);
    m_die1 = (forcedDie1 >= 1 && forcedDie1 <= 6) ? forcedDie1 : dist(m_rng);
    m_die2 = (forcedDie2 >= 1 && forcedDie2 <= 6) ? forcedDie2 : dist(m_rng);
    bool isDouble = (m_die1 == m_die2);

    Player& p = activePlayerMut();

    if (p.inJail) {
        if (isDouble) {
            p.inJail = false;
            p.jailTurns = 0;
            p.doublesCount = 0;
            m_canRollAgain = false; // Escaping jail via double does not grant another roll
            addHistory("mono_log_jail_double_escape", p.id, -1, -1, m_die1 + m_die2);
            moveActivePlayerBy(m_die1 + m_die2);
        } else {
            p.jailTurns += 1;
            if (p.jailTurns >= 3) {
                // Must pay 50M fine on 3rd failed attempt and move
                p.inJail = false;
                p.jailTurns = 0;
                p.doublesCount = 0;
                m_canRollAgain = false;
                addHistory("mono_log_paid_jail", p.id, -1, 10, 50);
                if (p.money >= 50) {
                    p.money -= 50;
                    recordTransfer(p.id, -1, 50, 10);
                    moveActivePlayerBy(m_die1 + m_die2);
                } else {
                    moveActivePlayerBy(m_die1 + m_die2);
                    requirePayment(p.id, -1, 50, 10);
                }
            } else {
                m_canRollAgain = false;
                addHistory("mono_log_stay_in_jail", p.id, -1, 10, m_die1 + m_die2);
                m_phase = TurnPhase::WaitingForEndTurn;
            }
        }
        return {m_die1, m_die2};
    }

    if (isDouble) {
        p.doublesCount += 1;
        if (p.doublesCount >= 3) {
            addHistory("mono_log_three_doubles_jail", p.id);
            sendActivePlayerToJail();
            return {m_die1, m_die2};
        }
        m_canRollAgain = true;
    } else {
        p.doublesCount = 0;
        m_canRollAgain = false;
    }

    moveActivePlayerBy(m_die1 + m_die2);
    return {m_die1, m_die2};
}

void MonopolyGame::moveActivePlayerBy(int steps) {
    Player& p = activePlayerMut();
    m_lastMovePath.clear();
    m_lastMovePath.push_back(p.position);

    int cur = p.position;
    bool passedGo = false;
    if (steps >= 0) {
        for (int i = 0; i < steps; ++i) {
            cur = (cur + 1) % MonopolyBoard::NUM_SPACES;
            m_lastMovePath.push_back(cur);
            if (cur == 0) {
                passedGo = true;
            }
        }
    } else {
        for (int i = 0; i < -steps; ++i) {
            cur = (cur - 1 + MonopolyBoard::NUM_SPACES) % MonopolyBoard::NUM_SPACES;
            m_lastMovePath.push_back(cur);
        }
    }

    p.position = cur;

    if (passedGo) {
        p.money += 200;
        recordTransfer(-1, p.id, 200, 0);
        addHistory("mono_log_passed_go", p.id, -1, 0, 200);
    }

    if (isDoubleRoll() && p.doublesCount > 0) {
        addHistory("mono_log_moved_double", p.id, -1, cur, diceSum());
    } else {
        addHistory("mono_log_moved", p.id, -1, cur, diceSum());
    }

    resolveLanding(cur, m_pendingRentMultiplier);
}

void MonopolyGame::moveActivePlayerTo(int targetSpace, bool collectGoIfPassed) {
    Player& p = activePlayerMut();
    m_lastMovePath.clear();
    m_lastMovePath.push_back(p.position);

    int cur = p.position;
    bool passedGo = false;
    while (cur != targetSpace) {
        cur = (cur + 1) % MonopolyBoard::NUM_SPACES;
        m_lastMovePath.push_back(cur);
        if (cur == 0 && collectGoIfPassed) {
            passedGo = true;
        }
    }
    p.position = targetSpace;

    if (passedGo) {
        p.money += 200;
        recordTransfer(-1, p.id, 200, 0);
        addHistory("mono_log_passed_go", p.id, -1, 0, 200);
    }

    addHistory("mono_log_moved", p.id, -1, targetSpace, 0);
    resolveLanding(targetSpace, m_pendingRentMultiplier);
}

void MonopolyGame::sendActivePlayerToJail() {
    Player& p = activePlayerMut();
    m_lastMovePath.clear();
    m_lastMovePath.push_back(p.position);
    m_lastMovePath.push_back(10);
    m_lastMoveWasJailSend = true;

    p.position = 10;
    p.inJail = true;
    p.jailTurns = 0;
    p.doublesCount = 0;
    m_canRollAgain = false;
    addHistory("mono_log_sent_to_jail", p.id, -1, 10);
    m_phase = TurnPhase::WaitingForEndTurn;
}

void MonopolyGame::resolveLanding(int spaceIndex, int rentMultiplier) {
    const Space& s = m_board.space(spaceIndex);
    Player& p = activePlayerMut();

    switch (s.type) {
        case SpaceType::Go:
        case SpaceType::Jail:
        case SpaceType::FreeParking:
            m_phase = TurnPhase::WaitingForEndTurn;
            break;

        case SpaceType::GoToJail:
            sendActivePlayerToJail();
            break;

        case SpaceType::Tax:
            addHistory("mono_log_paid_tax", p.id, -1, spaceIndex, s.taxAmount);
            requirePayment(p.id, -1, s.taxAmount, spaceIndex);
            break;

        case SpaceType::Chance:
            drawCard(true);
            break;

        case SpaceType::CommunityChest:
            drawCard(false);
            break;

        case SpaceType::Property:
        case SpaceType::Railroad:
        case SpaceType::Utility:
            if (s.ownerId == -1) {
                m_phase = TurnPhase::BuyOrAuction;
            } else if (s.ownerId != p.id && !s.mortgaged && !m_players[s.ownerId].bankrupt) {
                int rent = m_board.calculateRent(spaceIndex, diceSum(), rentMultiplier);
                if (rent > 0) {
                    m_rentNotif.fromPlayerId = p.id;
                    m_rentNotif.toPlayerId = s.ownerId;
                    m_rentNotif.spaceIndex = spaceIndex;
                    m_rentNotif.amount = rent;
                    m_phase = TurnPhase::RentPaymentModal;
                } else {
                    m_phase = TurnPhase::WaitingForEndTurn;
                }
            } else {
                m_phase = TurnPhase::WaitingForEndTurn;
            }
            break;
    }
}

bool MonopolyGame::buyCurrentProperty() {
    if (m_phase != TurnPhase::BuyOrAuction) return false;
    Player& p = activePlayerMut();
    int idx = p.position;
    Space& s = m_board.spaceMut(idx);
    if (!m_board.isPurchasable(idx) || s.ownerId != -1) return false;
    if (p.money < s.price) return false;

    p.money -= s.price;
    s.ownerId = p.id;
    recordTransfer(p.id, -1, s.price, idx);
    addHistory("mono_log_bought_property", p.id, -1, idx, s.price);
    m_phase = TurnPhase::WaitingForEndTurn;
    return true;
}

void MonopolyGame::startAuctionForCurrentProperty() {
    if (m_phase != TurnPhase::BuyOrAuction) return;
    int idx = activePlayer().position;
    if (!m_board.isPurchasable(idx)) return;

    m_auction.spaceIndex = idx;
    m_auction.currentBid = 0;
    m_auction.highestBidderId = -1;
    m_auction.activeBidderId = m_activePlayerIndex;
    m_auction.inputBid = 10;
    m_auction.folded.assign(m_players.size(), false);
    for (size_t i = 0; i < m_players.size(); ++i) {
        if (m_players[i].bankrupt || m_players[i].money < 1) {
            m_auction.folded[i] = true;
        }
    }

    m_phase = TurnPhase::Auction;
    addHistory("mono_log_auction_started", m_activePlayerIndex, -1, idx);

    if (m_auction.folded[m_auction.activeBidderId]) {
        advanceAuctionBidder();
    }
}

bool MonopolyGame::placeAuctionBid(int playerId, int bidAmount) {
    if (m_phase != TurnPhase::Auction || playerId != m_auction.activeBidderId) return false;
    if (bidAmount <= m_auction.currentBid || bidAmount > m_players[playerId].money) return false;

    m_auction.currentBid = bidAmount;
    m_auction.highestBidderId = playerId;
    m_auction.inputBid = bidAmount + 10;

    advanceAuctionBidder();
    return true;
}

bool MonopolyGame::foldAuction(int playerId) {
    if (m_phase != TurnPhase::Auction || playerId != m_auction.activeBidderId) return false;
    m_auction.folded[playerId] = true;
    advanceAuctionBidder();
    return true;
}

void MonopolyGame::advanceAuctionBidder() {
    int n = static_cast<int>(m_players.size());
    int activeCount = 0;
    int lastRemaining = -1;
    for (int i = 0; i < n; ++i) {
        if (!m_auction.folded[i] && m_players[i].money > m_auction.currentBid) {
            ++activeCount;
            lastRemaining = i;
        } else if (!m_auction.folded[i] && i == m_auction.highestBidderId) {
            ++activeCount;
            lastRemaining = i;
        } else {
            m_auction.folded[i] = true;
        }
    }

    if (activeCount == 0) {
        finalizeAuction();
        return;
    }
    if (activeCount == 1 && m_auction.highestBidderId == lastRemaining) {
        finalizeAuction();
        return;
    }

    int next = m_auction.activeBidderId;
    for (int step = 1; step <= n; ++step) {
        int cand = (m_auction.activeBidderId + step) % n;
        if (!m_auction.folded[cand] && cand != m_auction.highestBidderId) {
            next = cand;
            break;
        }
    }

    if (next == m_auction.highestBidderId || m_auction.folded[next]) {
        finalizeAuction();
        return;
    }

    m_auction.activeBidderId = next;
    m_auction.inputBid = std::min(m_players[next].money, std::max(m_auction.currentBid + 1, m_auction.currentBid + 10));
}

void MonopolyGame::finalizeAuction() {
    if (m_auction.highestBidderId >= 0 && m_auction.currentBid > 0) {
        Player& winner = m_players[m_auction.highestBidderId];
        Space& s = m_board.spaceMut(m_auction.spaceIndex);
        winner.money -= m_auction.currentBid;
        s.ownerId = winner.id;
        recordTransfer(winner.id, -1, m_auction.currentBid, m_auction.spaceIndex);
        addHistory("mono_log_auction_won", winner.id, -1, m_auction.spaceIndex, m_auction.currentBid);
    }
    m_phase = TurnPhase::WaitingForEndTurn;
}

void MonopolyGame::dismissRentModal() {
    if (m_phase != TurnPhase::RentPaymentModal) return;
    addHistory("mono_log_paid_rent", m_rentNotif.fromPlayerId, m_rentNotif.toPlayerId,
               m_rentNotif.spaceIndex, m_rentNotif.amount);
    requirePayment(m_rentNotif.fromPlayerId, m_rentNotif.toPlayerId,
                   m_rentNotif.amount, m_rentNotif.spaceIndex);
}

void MonopolyGame::drawCard(bool isChance) {
    if (isChance) {
        m_drawnCard = m_chanceDeck[m_chanceIndex % m_chanceDeck.size()];
        m_chanceIndex = (m_chanceIndex + 1) % m_chanceDeck.size();
    } else {
        m_drawnCard = m_chestDeck[m_chestIndex % m_chestDeck.size()];
        m_chestIndex = (m_chestIndex + 1) % m_chestDeck.size();
    }
    m_phase = TurnPhase::CardModal;
}

void MonopolyGame::dismissCardModal() {
    if (m_phase != TurnPhase::CardModal) return;
    applyDrawnCard();
}

void MonopolyGame::applyDrawnCard() {
    Player& p = activePlayerMut();
    const Card& c = m_drawnCard;

    switch (c.action) {
        case CardActionType::CollectMoney:
            p.money += c.amount;
            recordTransfer(-1, p.id, c.amount, p.position);
            addHistory("mono_log_card_collect", p.id, -1, p.position, c.amount);
            m_phase = TurnPhase::WaitingForEndTurn;
            break;

        case CardActionType::PayMoney:
            addHistory("mono_log_card_pay", p.id, -1, p.position, c.amount);
            requirePayment(p.id, -1, c.amount, p.position);
            break;

        case CardActionType::MoveTo:
            m_pendingRentMultiplier = 1;
            moveActivePlayerTo(c.targetSpace, true);
            break;

        case CardActionType::MoveRelative:
            m_pendingRentMultiplier = 1;
            moveActivePlayerBy(c.amount);
            break;

        case CardActionType::MoveToNearestRailroad: {
            int cur = p.position;
            int target = 5;
            if (cur < 5 || cur >= 35) target = 5;
            else if (cur < 15) target = 15;
            else if (cur < 25) target = 25;
            else target = 35;
            m_pendingRentMultiplier = 2; // Double rent on nearest railroad card
            moveActivePlayerTo(target, true);
            break;
        }

        case CardActionType::MoveToNearestUtility: {
            int cur = p.position;
            int target = (cur >= 12 && cur < 28) ? 28 : 12;
            m_pendingRentMultiplier = 2;
            moveActivePlayerTo(target, true);
            break;
        }

        case CardActionType::GoToJail:
            sendActivePlayerToJail();
            break;

        case CardActionType::GetOutOfJailFree:
            if (c.isChance) p.hasChanceJailCard = true;
            else p.hasChestJailCard = true;
            addHistory("mono_log_got_jail_card", p.id);
            m_phase = TurnPhase::WaitingForEndTurn;
            break;

        case CardActionType::StreetRepairs: {
            int houses = m_board.playerHouseCount(p.id);
            int hotels = m_board.playerHotelCount(p.id);
            int total = houses * c.perHouse + hotels * c.perHotel;
            if (total > 0) {
                addHistory("mono_log_card_pay", p.id, -1, p.position, total);
                requirePayment(p.id, -1, total, p.position);
            } else {
                m_phase = TurnPhase::WaitingForEndTurn;
            }
            break;
        }

        case CardActionType::PayEachPlayer: {
            int count = 0;
            for (const auto& other : m_players) {
                if (other.id != p.id && !other.bankrupt) ++count;
            }
            int total = count * c.amount;
            if (p.money >= total) {
                for (auto& other : m_players) {
                    if (other.id != p.id && !other.bankrupt) {
                        p.money -= c.amount;
                        other.money += c.amount;
                        recordTransfer(p.id, other.id, c.amount, p.position);
                    }
                }
                addHistory("mono_log_card_pay", p.id, -1, p.position, total);
                m_phase = TurnPhase::WaitingForEndTurn;
            } else {
                requirePayment(p.id, -1, total, p.position);
            }
            break;
        }

        case CardActionType::CollectFromEachPlayer: {
            int totalCollected = 0;
            for (auto& other : m_players) {
                if (other.id != p.id && !other.bankrupt) {
                    int amt = std::min(other.money, c.amount);
                    other.money -= amt;
                    p.money += amt;
                    totalCollected += amt;
                    recordTransfer(other.id, p.id, amt, p.position);
                }
            }
            addHistory("mono_log_card_collect", p.id, -1, p.position, totalCollected);
            m_phase = TurnPhase::WaitingForEndTurn;
            break;
        }
    }
}

void MonopolyGame::requirePayment(int payerId, int creditorId, int amount, int spaceIdx) {
    Player& payer = m_players[payerId];
    if (payer.money >= amount) {
        payer.money -= amount;
        if (creditorId >= 0 && creditorId < static_cast<int>(m_players.size())) {
            m_players[creditorId].money += amount;
        }
        recordTransfer(payerId, creditorId, amount, spaceIdx);
        m_pendingDebtAmount = 0;
        m_pendingCreditorId = -1;
        m_phase = TurnPhase::WaitingForEndTurn;
    } else {
        m_pendingDebtAmount = amount;
        m_pendingCreditorId = creditorId;
        m_phase = TurnPhase::DebtResolution;
    }
}

bool MonopolyGame::settlePendingDebt() {
    if (m_phase != TurnPhase::DebtResolution) return false;
    Player& payer = activePlayerMut();
    if (payer.money < m_pendingDebtAmount) return false;

    payer.money -= m_pendingDebtAmount;
    if (m_pendingCreditorId >= 0 && m_pendingCreditorId < static_cast<int>(m_players.size())) {
        m_players[m_pendingCreditorId].money += m_pendingDebtAmount;
    }
    recordTransfer(payer.id, m_pendingCreditorId, m_pendingDebtAmount, payer.position);
    m_pendingDebtAmount = 0;
    m_pendingCreditorId = -1;
    m_phase = TurnPhase::WaitingForEndTurn;
    return true;
}

int MonopolyGame::playerLiquidatableValue(int playerId) const {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return 0;
    int total = m_players[playerId].money;
    for (int idx : m_board.playerProperties(playerId)) {
        const Space& s = m_board.space(idx);
        if (s.buildings > 0) {
            total += s.buildings * (s.houseCost / 2);
        }
        if (!s.mortgaged) {
            total += s.mortgageValue;
        }
    }
    return total;
}

int MonopolyGame::playerNetWorth(int playerId) const {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return 0;
    int total = m_players[playerId].money;
    for (int idx : m_board.playerProperties(playerId)) {
        const Space& s = m_board.space(idx);
        total += s.mortgaged ? s.mortgageValue : s.price;
        total += s.buildings * s.houseCost;
    }
    return total;
}

void MonopolyGame::declareBankruptcy(int playerId) {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return;
    Player& loser = m_players[playerId];
    if (loser.bankrupt) return;

    int creditorId = (playerId == m_activePlayerIndex) ? m_pendingCreditorId : -1;

    // First sell all buildings back to Bank at half price
    for (int idx : m_board.playerProperties(playerId)) {
        Space& s = m_board.spaceMut(idx);
        if (s.buildings > 0) {
            loser.money += s.buildings * (s.houseCost / 2);
            s.buildings = 0;
        }
    }

    if (creditorId >= 0 && creditorId < static_cast<int>(m_players.size()) && !m_players[creditorId].bankrupt) {
        Player& cred = m_players[creditorId];
        if (loser.money > 0) {
            cred.money += loser.money;
            recordTransfer(playerId, creditorId, loser.money);
        }
        if (loser.hasChanceJailCard) cred.hasChanceJailCard = true;
        if (loser.hasChestJailCard) cred.hasChestJailCard = true;
        for (int idx : m_board.playerProperties(playerId)) {
            m_board.spaceMut(idx).ownerId = creditorId;
        }
    } else {
        // Bankrupt to Bank: release all properties
        for (int idx : m_board.playerProperties(playerId)) {
            Space& s = m_board.spaceMut(idx);
            s.ownerId = -1;
            s.buildings = 0;
            s.mortgaged = false;
        }
    }

    loser.money = 0;
    loser.hasChanceJailCard = false;
    loser.hasChestJailCard = false;
    loser.bankrupt = true;
    m_pendingDebtAmount = 0;
    m_pendingCreditorId = -1;

    addHistory("mono_log_bankrupt", playerId);
    checkVictory();
    if (m_status == GameStatus::InProgress && playerId == m_activePlayerIndex) {
        m_canRollAgain = false;
        endTurn();
    }
}

void MonopolyGame::checkVictory() {
    int aliveCount = 0;
    int lastAlive = -1;
    for (const auto& p : m_players) {
        if (!p.bankrupt) {
            ++aliveCount;
            lastAlive = p.id;
        }
    }
    if (aliveCount <= 1) {
        m_status = GameStatus::Finished;
        m_phase = TurnPhase::GameOver;
        m_winnerId = lastAlive;
    }
}

bool MonopolyGame::buildHouseOrHotel(int playerId, int spaceIndex) {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return false;
    Player& p = m_players[playerId];
    if (!m_board.canBuildOn(playerId, spaceIndex, p.money)) return false;

    const Space& s = m_board.space(spaceIndex);
    p.money -= s.houseCost;
    m_board.buildOn(playerId, spaceIndex);
    recordTransfer(playerId, -1, s.houseCost, spaceIndex);

    if (m_board.space(spaceIndex).buildings == 5) {
        addHistory("mono_log_built_hotel", playerId, -1, spaceIndex, s.houseCost);
    } else {
        addHistory("mono_log_built_house", playerId, -1, spaceIndex, s.houseCost);
    }
    return true;
}

bool MonopolyGame::sellHouseOrHotel(int playerId, int spaceIndex) {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return false;
    Player& p = m_players[playerId];
    if (!m_board.canSellBuildingOn(playerId, spaceIndex)) return false;

    const Space& s = m_board.space(spaceIndex);
    int refund = s.houseCost / 2;
    m_board.sellBuildingOn(playerId, spaceIndex);
    p.money += refund;
    recordTransfer(-1, playerId, refund, spaceIndex);
    addHistory("mono_log_sold_building", playerId, -1, spaceIndex, refund);

    if (m_phase == TurnPhase::DebtResolution && p.money >= m_pendingDebtAmount) {
        settlePendingDebt();
    }
    return true;
}

bool MonopolyGame::mortgageProperty(int playerId, int spaceIndex) {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return false;
    Player& p = m_players[playerId];
    if (!m_board.canMortgage(playerId, spaceIndex)) return false;

    int val = m_board.space(spaceIndex).mortgageValue;
    m_board.mortgageProperty(playerId, spaceIndex);
    p.money += val;
    recordTransfer(-1, playerId, val, spaceIndex);
    addHistory("mono_log_mortgaged", playerId, -1, spaceIndex, val);

    if (m_phase == TurnPhase::DebtResolution && p.money >= m_pendingDebtAmount) {
        settlePendingDebt();
    }
    return true;
}

bool MonopolyGame::unmortgageProperty(int playerId, int spaceIndex) {
    if (playerId < 0 || playerId >= static_cast<int>(m_players.size())) return false;
    Player& p = m_players[playerId];
    if (!m_board.canUnmortgage(playerId, spaceIndex, p.money)) return false;

    int cost = m_board.unmortgageCost(spaceIndex);
    p.money -= cost;
    m_board.unmortgageProperty(playerId, spaceIndex);
    recordTransfer(playerId, -1, cost, spaceIndex);
    addHistory("mono_log_unmortgaged", playerId, -1, spaceIndex, cost);
    return true;
}

bool MonopolyGame::executeTrade(const TradeOffer& offer) {
    if (offer.fromPlayerId < 0 || offer.fromPlayerId >= static_cast<int>(m_players.size())) return false;
    if (offer.toPlayerId < 0 || offer.toPlayerId >= static_cast<int>(m_players.size())) return false;
    if (offer.fromPlayerId == offer.toPlayerId) return false;

    Player& pFrom = m_players[offer.fromPlayerId];
    Player& pTo = m_players[offer.toPlayerId];

    if (pFrom.money < offer.offeredCash || pTo.money < offer.requestedCash) return false;
    if (pFrom.jailFreeCards() < offer.offeredJailCards || pTo.jailFreeCards() < offer.requestedJailCards) return false;

    // Verify properties belong to the respective players and have no buildings in their color group
    for (int idx : offer.offeredProperties) {
        const Space& s = m_board.space(idx);
        if (s.ownerId != pFrom.id) return false;
        if (s.type == SpaceType::Property && m_board.groupHasAnyBuildings(s.group)) return false;
    }
    for (int idx : offer.requestedProperties) {
        const Space& s = m_board.space(idx);
        if (s.ownerId != pTo.id) return false;
        if (s.type == SpaceType::Property && m_board.groupHasAnyBuildings(s.group)) return false;
    }

    // Transfer cash
    if (offer.offeredCash > 0) {
        pFrom.money -= offer.offeredCash;
        pTo.money += offer.offeredCash;
        recordTransfer(pFrom.id, pTo.id, offer.offeredCash);
    }
    if (offer.requestedCash > 0) {
        pTo.money -= offer.requestedCash;
        pFrom.money += offer.requestedCash;
        recordTransfer(pTo.id, pFrom.id, offer.requestedCash);
    }

    // Transfer jail cards
    for (int i = 0; i < offer.offeredJailCards; ++i) {
        if (pFrom.hasChanceJailCard) { pFrom.hasChanceJailCard = false; pTo.hasChanceJailCard = true; }
        else if (pFrom.hasChestJailCard) { pFrom.hasChestJailCard = false; pTo.hasChestJailCard = true; }
    }
    for (int i = 0; i < offer.requestedJailCards; ++i) {
        if (pTo.hasChanceJailCard) { pTo.hasChanceJailCard = false; pFrom.hasChanceJailCard = true; }
        else if (pTo.hasChestJailCard) { pTo.hasChestJailCard = false; pFrom.hasChestJailCard = true; }
    }

    // Transfer properties
    for (int idx : offer.offeredProperties) {
        m_board.spaceMut(idx).ownerId = pTo.id;
    }
    for (int idx : offer.requestedProperties) {
        m_board.spaceMut(idx).ownerId = pFrom.id;
    }

    addHistory("mono_log_trade_completed", pFrom.id, pTo.id);

    if (m_phase == TurnPhase::DebtResolution && activePlayer().money >= m_pendingDebtAmount) {
        settlePendingDebt();
    }
    return true;
}

void MonopolyGame::endTurn() {
    if (m_status != GameStatus::InProgress) return;
    if (m_phase == TurnPhase::WaitingForEndTurn && m_canRollAgain) {
        m_phase = TurnPhase::WaitingForRoll;
        return;
    }

    int n = static_cast<int>(m_players.size());
    for (int step = 1; step <= n; ++step) {
        int next = (m_activePlayerIndex + step) % n;
        if (!m_players[next].bankrupt) {
            m_activePlayerIndex = next;
            break;
        }
    }

    m_players[m_activePlayerIndex].doublesCount = 0;
    m_canRollAgain = false;
    m_phase = TurnPhase::WaitingForRoll;
}

} // namespace Strategix::Monopoly
