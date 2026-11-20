#include "MonopolyBoard.h"
#include <algorithm>

namespace Strategix::Monopoly {

MonopolyBoard::MonopolyBoard() {
    initSpaces();
}

void MonopolyBoard::reset() {
    initSpaces();
}

void MonopolyBoard::initSpaces() {
    auto makeProp = [](int idx, ColorGroup grp, int price, int houseCost,
                       int r0, int r1, int r2, int r3, int r4, int r5) {
        Space s;
        s.index = idx;
        s.type = SpaceType::Property;
        s.group = grp;
        s.nameKey = "mono_space_" + std::to_string(idx);
        s.price = price;
        s.houseCost = houseCost;
        s.mortgageValue = price / 2;
        s.rents = {r0, r1, r2, r3, r4, r5};
        return s;
    };

    auto makeSpecial = [](int idx, SpaceType type, ColorGroup grp = ColorGroup::None,
                          int price = 0, int tax = 0) {
        Space s;
        s.index = idx;
        s.type = type;
        s.group = grp;
        s.nameKey = "mono_space_" + std::to_string(idx);
        s.price = price;
        s.mortgageValue = price / 2;
        s.taxAmount = tax;
        return s;
    };

    // Bottom row (0 .. 10)
    m_spaces[0]  = makeSpecial(0, SpaceType::Go);
    m_spaces[1]  = makeProp(1, ColorGroup::Brown, 60, 50, 2, 10, 30, 90, 160, 250);
    m_spaces[2]  = makeSpecial(2, SpaceType::CommunityChest);
    m_spaces[3]  = makeProp(3, ColorGroup::Brown, 60, 50, 4, 20, 60, 180, 320, 450);
    m_spaces[4]  = makeSpecial(4, SpaceType::Tax, ColorGroup::None, 0, 200);
    m_spaces[5]  = makeSpecial(5, SpaceType::Railroad, ColorGroup::Railroad, 200);
    m_spaces[6]  = makeProp(6, ColorGroup::LightBlue, 100, 50, 6, 30, 90, 270, 400, 550);
    m_spaces[7]  = makeSpecial(7, SpaceType::Chance);
    m_spaces[8]  = makeProp(8, ColorGroup::LightBlue, 100, 50, 6, 30, 90, 270, 400, 550);
    m_spaces[9]  = makeProp(9, ColorGroup::LightBlue, 120, 50, 8, 40, 100, 300, 450, 600);
    m_spaces[10] = makeSpecial(10, SpaceType::Jail);

    // Left column (11 .. 20)
    m_spaces[11] = makeProp(11, ColorGroup::Pink, 140, 100, 10, 50, 150, 450, 625, 750);
    m_spaces[12] = makeSpecial(12, SpaceType::Utility, ColorGroup::Utility, 150);
    m_spaces[13] = makeProp(13, ColorGroup::Pink, 140, 100, 10, 50, 150, 450, 625, 750);
    m_spaces[14] = makeProp(14, ColorGroup::Pink, 160, 100, 12, 60, 180, 500, 700, 900);
    m_spaces[15] = makeSpecial(15, SpaceType::Railroad, ColorGroup::Railroad, 200);
    m_spaces[16] = makeProp(16, ColorGroup::Orange, 180, 100, 14, 70, 200, 550, 750, 950);
    m_spaces[17] = makeSpecial(17, SpaceType::CommunityChest);
    m_spaces[18] = makeProp(18, ColorGroup::Orange, 180, 100, 14, 70, 200, 550, 750, 950);
    m_spaces[19] = makeProp(19, ColorGroup::Orange, 200, 100, 16, 80, 220, 600, 800, 1000);
    m_spaces[20] = makeSpecial(20, SpaceType::FreeParking);

    // Top row (21 .. 30)
    m_spaces[21] = makeProp(21, ColorGroup::Red, 220, 150, 18, 90, 250, 700, 875, 1050);
    m_spaces[22] = makeSpecial(22, SpaceType::Chance);
    m_spaces[23] = makeProp(23, ColorGroup::Red, 220, 150, 18, 90, 250, 700, 875, 1050);
    m_spaces[24] = makeProp(24, ColorGroup::Red, 240, 150, 20, 100, 300, 750, 925, 1100);
    m_spaces[25] = makeSpecial(25, SpaceType::Railroad, ColorGroup::Railroad, 200);
    m_spaces[26] = makeProp(26, ColorGroup::Yellow, 260, 150, 22, 110, 330, 800, 975, 1150);
    m_spaces[27] = makeProp(27, ColorGroup::Yellow, 260, 150, 22, 110, 330, 800, 975, 1150);
    m_spaces[28] = makeSpecial(28, SpaceType::Utility, ColorGroup::Utility, 150);
    m_spaces[29] = makeProp(29, ColorGroup::Yellow, 280, 150, 24, 120, 360, 850, 1025, 1200);
    m_spaces[30] = makeSpecial(30, SpaceType::GoToJail);

    // Right column (31 .. 39)
    m_spaces[31] = makeProp(31, ColorGroup::Green, 300, 200, 26, 130, 390, 900, 1100, 1275);
    m_spaces[32] = makeProp(32, ColorGroup::Green, 300, 200, 26, 130, 390, 900, 1100, 1275);
    m_spaces[33] = makeSpecial(33, SpaceType::CommunityChest);
    m_spaces[34] = makeProp(34, ColorGroup::Green, 320, 200, 28, 150, 450, 1000, 1200, 1400);
    m_spaces[35] = makeSpecial(35, SpaceType::Railroad, ColorGroup::Railroad, 200);
    m_spaces[36] = makeSpecial(36, SpaceType::Chance);
    m_spaces[37] = makeProp(37, ColorGroup::DarkBlue, 350, 200, 35, 175, 500, 1100, 1300, 1500);
    m_spaces[38] = makeSpecial(38, SpaceType::Tax, ColorGroup::None, 0, 100);
    m_spaces[39] = makeProp(39, ColorGroup::DarkBlue, 400, 200, 50, 200, 600, 1400, 1700, 2000);
}

const Space& MonopolyBoard::space(int index) const {
    return m_spaces[index % NUM_SPACES];
}

Space& MonopolyBoard::spaceMut(int index) {
    return m_spaces[index % NUM_SPACES];
}

bool MonopolyBoard::isPurchasable(int index) const {
    if (index < 0 || index >= NUM_SPACES) return false;
    SpaceType t = m_spaces[index].type;
    return t == SpaceType::Property || t == SpaceType::Railroad || t == SpaceType::Utility;
}

std::vector<int> MonopolyBoard::groupSpaces(ColorGroup group) const {
    std::vector<int> res;
    if (group == ColorGroup::None) return res;
    for (int i = 0; i < NUM_SPACES; ++i) {
        if (m_spaces[i].group == group) {
            res.push_back(i);
        }
    }
    return res;
}

std::vector<int> MonopolyBoard::playerProperties(int playerId) const {
    std::vector<int> res;
    for (int i = 0; i < NUM_SPACES; ++i) {
        if (m_spaces[i].ownerId == playerId) {
            res.push_back(i);
        }
    }
    return res;
}

int MonopolyBoard::countOwnedInGroup(int playerId, ColorGroup group) const {
    if (playerId < 0 || group == ColorGroup::None) return 0;
    int count = 0;
    for (int idx : groupSpaces(group)) {
        if (m_spaces[idx].ownerId == playerId) {
            ++count;
        }
    }
    return count;
}

bool MonopolyBoard::ownsFullColorGroup(int playerId, ColorGroup group) const {
    if (playerId < 0 || group == ColorGroup::None) return false;
    auto indices = groupSpaces(group);
    if (indices.empty()) return false;
    for (int idx : indices) {
        if (m_spaces[idx].ownerId != playerId) return false;
    }
    return true;
}

bool MonopolyBoard::groupHasAnyMortgaged(ColorGroup group) const {
    for (int idx : groupSpaces(group)) {
        if (m_spaces[idx].mortgaged) return true;
    }
    return false;
}

bool MonopolyBoard::groupHasAnyBuildings(ColorGroup group) const {
    for (int idx : groupSpaces(group)) {
        if (m_spaces[idx].buildings > 0) return true;
    }
    return false;
}

int MonopolyBoard::calculateRent(int spaceIndex, int diceSum, int rentMultiplier) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return 0;
    const Space& s = m_spaces[spaceIndex];
    if (s.ownerId < 0 || s.mortgaged) return 0;

    if (s.type == SpaceType::Property) {
        if (s.buildings == 0) {
            int baseRent = s.rents[0];
            if (ownsFullColorGroup(s.ownerId, s.group)) {
                return baseRent * 2 * rentMultiplier;
            }
            return baseRent * rentMultiplier;
        }
        int b = std::clamp(s.buildings, 1, 5);
        return s.rents[b] * rentMultiplier;
    } else if (s.type == SpaceType::Railroad) {
        int owned = countOwnedInGroup(s.ownerId, ColorGroup::Railroad);
        if (owned <= 0) return 0;
        int rent = 25 * (1 << (owned - 1));
        return rent * rentMultiplier;
    } else if (s.type == SpaceType::Utility) {
        int owned = countOwnedInGroup(s.ownerId, ColorGroup::Utility);
        if (owned <= 0) return 0;
        int factor = (rentMultiplier > 1) ? 10 : ((owned >= 2) ? 10 : 4);
        return diceSum * factor;
    }
    return 0;
}

int MonopolyBoard::bankHousesAvailable() const {
    int used = 0;
    for (const auto& s : m_spaces) {
        if (s.buildings >= 1 && s.buildings <= 4) {
            used += s.buildings;
        }
    }
    return std::max(0, MAX_BANK_HOUSES - used);
}

int MonopolyBoard::bankHotelsAvailable() const {
    int used = 0;
    for (const auto& s : m_spaces) {
        if (s.buildings == 5) {
            ++used;
        }
    }
    return std::max(0, MAX_BANK_HOTELS - used);
}

int MonopolyBoard::playerHouseCount(int playerId) const {
    int count = 0;
    for (const auto& s : m_spaces) {
        if (s.ownerId == playerId && s.buildings >= 1 && s.buildings <= 4) {
            count += s.buildings;
        }
    }
    return count;
}

int MonopolyBoard::playerHotelCount(int playerId) const {
    int count = 0;
    for (const auto& s : m_spaces) {
        if (s.ownerId == playerId && s.buildings == 5) {
            ++count;
        }
    }
    return count;
}

bool MonopolyBoard::canBuildOn(int playerId, int spaceIndex, int playerCash) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    const Space& s = m_spaces[spaceIndex];
    if (s.type != SpaceType::Property || s.ownerId != playerId || s.mortgaged) return false;
    if (s.buildings >= 5) return false;
    if (playerCash < s.houseCost) return false;
    if (!ownsFullColorGroup(playerId, s.group)) return false;
    if (groupHasAnyMortgaged(s.group)) return false;

    // Equal building rule: can only build on properties with the minimum building count in the group
    int minB = 5;
    for (int idx : groupSpaces(s.group)) {
        minB = std::min(minB, m_spaces[idx].buildings);
    }
    if (s.buildings != minB) return false;

    if (s.buildings == 4) {
        return bankHotelsAvailable() > 0;
    } else {
        return bankHousesAvailable() > 0;
    }
}

bool MonopolyBoard::canSellBuildingOn(int playerId, int spaceIndex) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    const Space& s = m_spaces[spaceIndex];
    if (s.type != SpaceType::Property || s.ownerId != playerId) return false;
    if (s.buildings <= 0) return false;

    // Equal selling rule: can only sell from properties with the maximum building count in the group
    int maxB = 0;
    for (int idx : groupSpaces(s.group)) {
        maxB = std::max(maxB, m_spaces[idx].buildings);
    }
    return s.buildings == maxB;
}

bool MonopolyBoard::canMortgage(int playerId, int spaceIndex) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    const Space& s = m_spaces[spaceIndex];
    if (!isPurchasable(spaceIndex) || s.ownerId != playerId || s.mortgaged) return false;
    if (s.type == SpaceType::Property && groupHasAnyBuildings(s.group)) return false;
    return true;
}

int MonopolyBoard::unmortgageCost(int spaceIndex) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return 0;
    int m = m_spaces[spaceIndex].mortgageValue;
    int interest = (m + 9) / 10; // 10% rounded up
    return m + interest;
}

bool MonopolyBoard::canUnmortgage(int playerId, int spaceIndex, int playerCash) const {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    const Space& s = m_spaces[spaceIndex];
    if (!isPurchasable(spaceIndex) || s.ownerId != playerId || !s.mortgaged) return false;
    return playerCash >= unmortgageCost(spaceIndex);
}

bool MonopolyBoard::buildOn(int playerId, int spaceIndex) {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    Space& s = m_spaces[spaceIndex];
    if (s.ownerId != playerId || s.buildings >= 5) return false;
    s.buildings += 1;
    return true;
}

bool MonopolyBoard::sellBuildingOn(int playerId, int spaceIndex) {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    Space& s = m_spaces[spaceIndex];
    if (s.ownerId != playerId || s.buildings <= 0) return false;
    s.buildings -= 1;
    return true;
}

bool MonopolyBoard::mortgageProperty(int playerId, int spaceIndex) {
    if (!canMortgage(playerId, spaceIndex)) return false;
    m_spaces[spaceIndex].mortgaged = true;
    return true;
}

bool MonopolyBoard::unmortgageProperty(int playerId, int spaceIndex) {
    if (spaceIndex < 0 || spaceIndex >= NUM_SPACES) return false;
    Space& s = m_spaces[spaceIndex];
    if (s.ownerId != playerId || !s.mortgaged) return false;
    s.mortgaged = false;
    return true;
}

} // namespace Strategix::Monopoly
