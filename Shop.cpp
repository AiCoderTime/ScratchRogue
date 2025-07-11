#include "Shop.h"
#include "Utils.h"
#include <cstdlib>

// Populate shop with new random relic and card offers
void Shop::generateNewShop() {
    relicOffers.clear();
    cardOffers.clear();

    // Offer 2 relics and 3 scratch cards
    for (int i = 0; i < 2; ++i)
        relicOffers.push_back(generateRandomRelic());

    for (int i = 0; i < 3; ++i)
        cardOffers.push_back(generateRandomCard());
}

// Double the reroll cost and refresh the shop
void Shop::reroll() {
    rerollCost *= 2;
    generateNewShop();
}

// Accessors
int Shop::getRerollCost() const { return rerollCost; }
const std::vector<Relic>& Shop::getRelics() const { return relicOffers; }
const std::vector<ScratchCardOffer>& Shop::getScratchCards() const { return cardOffers; }

// Return a random relic from a predefined pool
Relic Shop::generateRandomRelic() {
    static const std::vector<Relic> relicPool = {
        {"lucky_coin", "Lucky Coin", "Increases winnings by 10%", 50},
        {"golden_ticket", "Golden Ticket", "Unlocks rare cards", 100},
        {"mystery_box", "Mystery Box", "Random effect each game", 75}
    };
    return relicPool[rand() % relicPool.size()];
}

// Return a random scratch card offer from a predefined pool
ScratchCardOffer Shop::generateRandomCard() {
    static const std::vector<ScratchCardOffer> cardPool = {
        {"Lucky 7's", Rarity::Common, {CardType::Lucky, CardType::Numeric}, 10, sf::Sprite(),
         { SymbolType::Seven, SymbolType::FourLeafClover, SymbolType::Horseshoe }},
        {"Gilded Fortune", Rarity::Rare, {CardType::Gilded, CardType::Lucky}, 25, sf::Sprite(),
         { SymbolType::Crown, SymbolType::Scepter, SymbolType::GildedCoin }},
        {"Royal Riches", Rarity::Rare, {CardType::Royal, CardType::Gilded}, 25, sf::Sprite(),
         { SymbolType::KingsCrown, SymbolType::QueensScepter, SymbolType::RoyalSeal }},
        {"Risky Business", Rarity::Uncommon, {CardType::Risky, CardType::Tricky}, 15, sf::Sprite(),
         { SymbolType::Skull, SymbolType::SnakeEyes, SymbolType::RouletteWheel }},
        {"Tricky Treat", Rarity::Uncommon, {CardType::Tricky, CardType::Lucky}, 15, sf::Sprite(),
         { SymbolType::JestersHat, SymbolType::MagicWand, SymbolType::Confetti }},
        {"Volatile Vault", Rarity::Epic, {CardType::Volatile, CardType::Gilded, CardType::Risky}, 50, sf::Sprite(),
         { SymbolType::Explosive, SymbolType::Lava, SymbolType::Electricity }},
        {"Experimental Edge", Rarity::Epic, {CardType::Experimental, CardType::Tricky, CardType::Lucky}, 50, sf::Sprite(),
         { SymbolType::TestTube, SymbolType::Beaker, SymbolType::LabCoat }},
        {"King's Bounty", Rarity::Legendary, {CardType::Royal, CardType::Gilded, CardType::Lucky}, 100, sf::Sprite(),
         { SymbolType::KingsScepter, SymbolType::RoyalCrown, SymbolType::GoldenCoin }},
        {"Golden Gamble", Rarity::Legendary, {CardType::Gilded, CardType::Risky, CardType::Volatile}, 100, sf::Sprite(),
         { SymbolType::GoldenChip, SymbolType::PokerCards, SymbolType::Dice }}
    };

    // Pick random card offer
    int idx = Utils::randInt(0, static_cast<int>(cardPool.size()) - 1);
    return cardPool[idx];
}
