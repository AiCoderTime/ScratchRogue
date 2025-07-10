#include "Shop.h"
#include "Utils.h"
#include <cstdlib>

void Shop::generateNewShop() {
	relicOffers.clear();
	cardOffers.clear();
	for (int i = 0; i < 2; ++i) relicOffers.push_back(generateRandomRelic());
	for (int i = 0; i < 3; ++i) cardOffers.push_back(generateRandomCard());
}

void Shop::reroll() {
	rerollCost *= 2;
	generateNewShop();
}

int Shop::getRerollCost() const { return rerollCost; }
const std::vector<Relic>& Shop::getRelics() const { return relicOffers; }
const std::vector<ScratchCardOffer>& Shop::getScratchCards() const { return cardOffers; }

Relic Shop::generateRandomRelic() {
    static const std::vector<Relic> relicPool = {
        {"lucky_coin", "Lucky Coin", "Increases winnings by 10%", 50},
        {"golden_ticket", "Golden Ticket", "Unlocks rare cards", 100},
        {"mystery_box", "Mystery Box", "Random effect each game", 75}
    };
    return relicPool[rand() % relicPool.size()];
}

ScratchCardOffer Shop::generateRandomCard() {
    // Define the card pool with lore-appropriate names and card types
    static const std::vector<ScratchCardOffer> cardPool = {
        {"Lucky 7's", Rarity::Common, {CardType::Lucky, CardType::Numeric}, 10, sf::Sprite()},
        {"Gilded Fortune", Rarity::Rare, {CardType::Gilded, CardType::Lucky}, 25, sf::Sprite()},
        {"Royal Riches", Rarity::Rare, {CardType::Royal, CardType::Gilded}, 25, sf::Sprite()},
        {"Risky Business", Rarity::Uncommon, {CardType::Risky, CardType::Tricky}, 15, sf::Sprite()},
        {"Tricky Treat", Rarity::Uncommon, {CardType::Tricky, CardType::Lucky}, 15, sf::Sprite()},
        {"Volatile Vault", Rarity::Epic, {CardType::Volatile, CardType::Gilded, CardType::Risky}, 50, sf::Sprite()},
        {"Experimental Edge", Rarity::Epic, {CardType::Experimental, CardType::Tricky, CardType::Lucky}, 50, sf::Sprite()},
        {"King's Bounty", Rarity::Legendary, {CardType::Royal, CardType::Gilded, CardType::Lucky}, 100, sf::Sprite()},
        {"Golden Gamble", Rarity::Legendary, {CardType::Gilded, CardType::Risky, CardType::Volatile}, 100, sf::Sprite()}
    };

    // Use your utils for random number generation
    int idx = Utils::randInt(0, static_cast<int>(cardPool.size()) - 1);
    return cardPool[idx];
}