#pragma once
#include <vector>
#include "Relic.h"
#include "ScratchCardOffer.h"

class Shop {
public:
    // Generate a new shop inventory of relics and scratch cards
    void generateNewShop();

    // Double reroll cost and refresh shop offers
    void reroll();

    // Accessors for current shop offers
    const std::vector<Relic>& getRelics() const;
    const std::vector<ScratchCardOffer>& getScratchCards() const;
    int getRerollCost() const;

private:
    std::vector<Relic> relicOffers;           // Currently offered relics
    std::vector<ScratchCardOffer> cardOffers; // Currently offered cards
    int rerollCost = 10;                       // Cost to reroll shop offers

    // Helpers to generate random relics and cards
    Relic generateRandomRelic();
    ScratchCardOffer generateRandomCard();
};
