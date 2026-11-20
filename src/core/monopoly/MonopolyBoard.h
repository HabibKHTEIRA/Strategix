#ifndef STRATEGIX_MONOPOLYBOARD_H
#define STRATEGIX_MONOPOLYBOARD_H

#include <array>
#include <string>
#include <vector>

namespace Strategix::Monopoly {

enum class SpaceType {
    Go,
    Property,
    CommunityChest,
    Chance,
    Tax,
    Railroad,
    Utility,
    Jail,
    FreeParking,
    GoToJail
};

enum class ColorGroup {
    None,
    Brown,
    LightBlue,
    Pink,
    Orange,
    Red,
    Yellow,
    Green,
    DarkBlue,
    Railroad,
    Utility
};

struct Space {
    int index = 0;
    SpaceType type = SpaceType::Go;
    ColorGroup group = ColorGroup::None;
    std::string nameKey;
    int price = 0;
    int houseCost = 0;
    int mortgageValue = 0;
    std::array<int, 6> rents = {0, 0, 0, 0, 0, 0}; // 0: base, 1..4: houses, 5: hotel
    int taxAmount = 0;

    // Runtime state
    int ownerId = -1;     // -1 = Bank
    int buildings = 0;    // 0..4 = houses, 5 = hotel
    bool mortgaged = false;
};

class MonopolyBoard {
public:
    static constexpr int NUM_SPACES = 40;
    static constexpr int MAX_BANK_HOUSES = 32;
    static constexpr int MAX_BANK_HOTELS = 12;

    MonopolyBoard();

    void reset();

    const Space& space(int index) const;
    Space& spaceMut(int index);
    const std::array<Space, NUM_SPACES>& spaces() const { return m_spaces; }

    bool isPurchasable(int index) const;
    std::vector<int> groupSpaces(ColorGroup group) const;
    std::vector<int> playerProperties(int playerId) const;

    bool ownsFullColorGroup(int playerId, ColorGroup group) const;
    bool groupHasAnyMortgaged(ColorGroup group) const;
    bool groupHasAnyBuildings(ColorGroup group) const;
    int countOwnedInGroup(int playerId, ColorGroup group) const;

    int calculateRent(int spaceIndex, int diceSum, int rentMultiplier = 1) const;

    int bankHousesAvailable() const;
    int bankHotelsAvailable() const;
    int playerHouseCount(int playerId) const;
    int playerHotelCount(int playerId) const;

    bool canBuildOn(int playerId, int spaceIndex, int playerCash) const;
    bool canSellBuildingOn(int playerId, int spaceIndex) const;
    bool canMortgage(int playerId, int spaceIndex) const;
    bool canUnmortgage(int playerId, int spaceIndex, int playerCash) const;
    int unmortgageCost(int spaceIndex) const;

    bool buildOn(int playerId, int spaceIndex);
    bool sellBuildingOn(int playerId, int spaceIndex);
    bool mortgageProperty(int playerId, int spaceIndex);
    bool unmortgageProperty(int playerId, int spaceIndex);

private:
    void initSpaces();
    std::array<Space, NUM_SPACES> m_spaces;
};

} // namespace Strategix::Monopoly

#endif // STRATEGIX_MONOPOLYBOARD_H
