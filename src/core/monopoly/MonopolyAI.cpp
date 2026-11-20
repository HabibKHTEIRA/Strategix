#include "MonopolyAI.h"
#include <algorithm>

namespace Strategix::Monopoly {

int MonopolyAI::safetyCashReserve(AIDifficulty diff) const {
    switch (diff) {
        case AIDifficulty::Easy:   return 260;
        case AIDifficulty::Medium: return 170;
        case AIDifficulty::Hard:   return 120;
    }
    return 170;
}

int MonopolyAI::evaluatePropertyValue(const MonopolyGame& game, int playerId, int spaceIndex) const {
    const MonopolyBoard& board = game.board();
    if (!board.isPurchasable(spaceIndex)) return 0;
    const Space& s = board.space(spaceIndex);
    int val = s.price;

    AIDifficulty diff = game.player(playerId).difficulty;

    int myOwned = board.countOwnedInGroup(playerId, s.group);
    auto grpSpaces = board.groupSpaces(s.group);
    int grpSize = static_cast<int>(grpSpaces.size());

    // Bonus if we already own properties in this group
    if (s.type == SpaceType::Property && grpSize > 0) {
        if (myOwned == grpSize - 1) {
            // Completes our monopoly!
            val += (diff == AIDifficulty::Hard) ? (s.price * 3 / 4) : (s.price / 2);
        } else if (myOwned > 0) {
            val += s.price / 4;
        }

        // Check if an opponent is 1 short of completing this monopoly
        if (diff != AIDifficulty::Easy) {
            for (const auto& other : game.players()) {
                if (other.id != playerId && !other.bankrupt) {
                    if (board.countOwnedInGroup(other.id, s.group) == grpSize - 1) {
                        val += (diff == AIDifficulty::Hard) ? (s.price * 2 / 3) : (s.price / 3);
                    }
                }
            }
        }

        // High-traffic color groups (Orange, Red, LightBlue, Yellow) bonus for Hard AI
        if (diff == AIDifficulty::Hard) {
            if (s.group == ColorGroup::Orange || s.group == ColorGroup::Red) {
                val += 40;
            } else if (s.group == ColorGroup::LightBlue || s.group == ColorGroup::Yellow) {
                val += 25;
            }
        }
    } else if (s.type == SpaceType::Railroad) {
        val += myOwned * 45;
    } else if (s.type == SpaceType::Utility) {
        val += myOwned * 35;
    }

    if (s.mortgaged) {
        val -= s.mortgageValue;
    }

    return val;
}

bool MonopolyAI::shouldPayToLeaveJail(const MonopolyGame& game, int playerId) const {
    const Player& p = game.player(playerId);
    if (!p.inJail) return false;

    // Count how many purchasable properties are still unowned
    int unowned = 0;
    for (int i = 0; i < MonopolyBoard::NUM_SPACES; ++i) {
        if (game.board().isPurchasable(i) && game.board().space(i).ownerId == -1) {
            ++unowned;
        }
    }

    // Early/mid game: get out of jail quickly to buy properties!
    if (unowned >= 8) {
        return p.jailFreeCards() > 0 || p.money >= 200;
    }
    // Late game: stay in jail to avoid paying high rents on opponent hotels
    return false;
}

bool MonopolyAI::shouldBuyProperty(const MonopolyGame& game, int playerId, int spaceIndex) const {
    const Player& p = game.player(playerId);
    const Space& s = game.board().space(spaceIndex);
    if (p.money < s.price) return false;

    int val = evaluatePropertyValue(game, playerId, spaceIndex);
    int reserve = safetyCashReserve(p.difficulty);

    // Always buy if it completes a color group and we can afford it
    auto grpSpaces = game.board().groupSpaces(s.group);
    if (!grpSpaces.empty() && game.board().countOwnedInGroup(playerId, s.group) == static_cast<int>(grpSpaces.size()) - 1) {
        return true;
    }

    if (p.money - s.price >= reserve) {
        return true;
    }

    // Even if below reserve, Medium/Hard will buy if value is significantly higher than price
    if (p.difficulty != AIDifficulty::Easy && val >= s.price + 40 && p.money >= s.price + 40) {
        return true;
    }

    return false;
}

int MonopolyAI::chooseAuctionBid(const MonopolyGame& game, int playerId) const {
    const AuctionState& auc = game.auctionState();
    const Player& p = game.player(playerId);
    if (auc.spaceIndex < 0) return -1;

    int val = evaluatePropertyValue(game, playerId, auc.spaceIndex);
    int reserve = safetyCashReserve(p.difficulty) / 2;
    int maxAffordable = std::max(0, p.money - reserve);

    int maxBid = val;
    if (p.difficulty == AIDifficulty::Easy) {
        maxBid = val * 80 / 100;
    } else if (p.difficulty == AIDifficulty::Medium) {
        maxBid = val;
    } else {
        maxBid = val * 115 / 100;
    }
    maxBid = std::min(maxBid, maxAffordable);

    int nextBid = auc.currentBid + 10;
    if (nextBid <= maxBid && nextBid <= p.money) {
        return nextBid;
    }
    if (auc.currentBid + 1 <= maxBid && auc.currentBid + 1 <= p.money) {
        return auc.currentBid + 1;
    }
    return -1; // Fold
}

void MonopolyAI::performTurnManagement(MonopolyGame& game, int playerId) const {
    const Player& p = game.player(playerId);
    int reserve = safetyCashReserve(p.difficulty);

    // 1. Unmortgage properties if we have plenty of cash
    for (int idx : game.board().playerProperties(playerId)) {
        const Space& s = game.board().space(idx);
        if (s.mortgaged) {
            int cost = game.board().unmortgageCost(idx);
            if (game.player(playerId).money - cost >= reserve + 100) {
                game.unmortgageProperty(playerId, idx);
            }
        }
    }

    // 2. Build houses/hotels on monopolies while respecting safety reserve
    bool builtAny = true;
    int maxBuildsThisTurn = (p.difficulty == AIDifficulty::Easy) ? 2 : 6;
    int buildsCount = 0;
    while (builtAny && buildsCount < maxBuildsThisTurn) {
        builtAny = false;
        int bestIdx = -1;
        int bestScore = -1;

        for (int idx : game.board().playerProperties(playerId)) {
            if (game.board().canBuildOn(playerId, idx, game.player(playerId).money)) {
                const Space& s = game.board().space(idx);
                if (game.player(playerId).money - s.houseCost >= reserve) {
                    int nextB = std::min(5, s.buildings + 1);
                    int rentGain = s.rents[nextB] - s.rents[s.buildings];
                    if (rentGain > bestScore) {
                        bestScore = rentGain;
                        bestIdx = idx;
                    }
                }
            }
        }

        if (bestIdx != -1) {
            builtAny = game.buildHouseOrHotel(playerId, bestIdx);
            if (builtAny) ++buildsCount;
        }
    }
}

void MonopolyAI::resolveDebt(MonopolyGame& game, int playerId) const {
    if (game.phase() != TurnPhase::DebtResolution) return;

    // 1. Mortgage unimproved standalone properties first, then monopoly properties
    for (int pass = 0; pass < 2 && game.phase() == TurnPhase::DebtResolution; ++pass) {
        for (int idx : game.board().playerProperties(playerId)) {
            const Space& s = game.board().space(idx);
            bool isMonopoly = game.board().ownsFullColorGroup(playerId, s.group);
            if ((pass == 0 && !isMonopoly) || pass == 1) {
                if (game.board().canMortgage(playerId, idx)) {
                    game.mortgageProperty(playerId, idx);
                    if (game.phase() != TurnPhase::DebtResolution) return;
                }
            }
        }
    }

    // 2. Sell houses/hotels and then mortgage if still in debt
    int safetyLoop = 0;
    while (game.phase() == TurnPhase::DebtResolution && safetyLoop < 100) {
        ++safetyLoop;
        bool actionTaken = false;
        for (int idx : game.board().playerProperties(playerId)) {
            if (game.board().canSellBuildingOn(playerId, idx)) {
                game.sellHouseOrHotel(playerId, idx);
                actionTaken = true;
                if (game.phase() != TurnPhase::DebtResolution) return;
                break;
            }
        }
        if (!actionTaken) {
            for (int idx : game.board().playerProperties(playerId)) {
                if (game.board().canMortgage(playerId, idx)) {
                    game.mortgageProperty(playerId, idx);
                    actionTaken = true;
                    if (game.phase() != TurnPhase::DebtResolution) return;
                    break;
                }
            }
        }
        if (!actionTaken) break;
    }

    if (game.phase() == TurnPhase::DebtResolution) {
        if (!game.settlePendingDebt()) {
            game.declareBankruptcy(playerId);
        }
    }
}

bool MonopolyAI::evaluateTradeOffer(const MonopolyGame& game, const TradeOffer& offer) const {
    int aiId = offer.toPlayerId;
    int humanId = offer.fromPlayerId;
    const Player& ai = game.player(aiId);

    int valueReceived = offer.offeredCash + offer.offeredJailCards * 50;
    for (int idx : offer.offeredProperties) {
        valueReceived += evaluatePropertyValue(game, aiId, idx);
    }

    int valueGiven = offer.requestedCash + offer.requestedJailCards * 50;
    for (int idx : offer.requestedProperties) {
        int myVal = evaluatePropertyValue(game, aiId, idx);
        int theirVal = evaluatePropertyValue(game, humanId, idx);
        valueGiven += std::max(myVal, theirVal);
    }

    if (valueReceived == 0 && valueGiven == 0) return false;

    if (ai.difficulty == AIDifficulty::Easy) {
        return valueReceived >= valueGiven * 90 / 100;
    } else if (ai.difficulty == AIDifficulty::Medium) {
        return valueReceived >= valueGiven;
    } else {
        return valueReceived >= valueGiven * 110 / 100;
    }
}

} // namespace Strategix::Monopoly
