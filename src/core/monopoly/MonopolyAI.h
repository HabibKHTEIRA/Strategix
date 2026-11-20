#ifndef STRATEGIX_MONOPOLYAI_H
#define STRATEGIX_MONOPOLYAI_H

#include "MonopolyGame.h"

namespace Strategix::Monopoly {

class MonopolyAI {
public:
    MonopolyAI() = default;

    void setDifficulty(AIDifficulty diff) { m_difficulty = diff; }
    AIDifficulty difficulty() const { return m_difficulty; }

    // Decision methods for AI turn phases
    bool shouldPayToLeaveJail(const MonopolyGame& game, int playerId) const;
    bool shouldBuyProperty(const MonopolyGame& game, int playerId, int spaceIndex) const;
    int chooseAuctionBid(const MonopolyGame& game, int playerId) const;
    void performTurnManagement(MonopolyGame& game, int playerId) const;
    void resolveDebt(MonopolyGame& game, int playerId) const;
    bool evaluateTradeOffer(const MonopolyGame& game, const TradeOffer& offer) const;

    int evaluatePropertyValue(const MonopolyGame& game, int playerId, int spaceIndex) const;

private:
    int safetyCashReserve(AIDifficulty diff) const;
    AIDifficulty m_difficulty = AIDifficulty::Medium;
};

} // namespace Strategix::Monopoly

#endif // STRATEGIX_MONOPOLYAI_H
