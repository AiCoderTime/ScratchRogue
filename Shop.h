#pragma once
#include <vector>
#include "Relic.h"
#include "ScratchCardOffer.h"

class Shop {
public:
	void generateNewShop();
	void reroll();

	const std::vector<Relic>& getRelics() const;
	const std::vector<ScratchCardOffer>& getScratchCards() const;
	int getRerollCost() const;

private:
	std::vector<Relic> relicOffers;
	std::vector<ScratchCardOffer> cardOffers;
	int rerollCost = 10;

	Relic generateRandomRelic();
	ScratchCardOffer generateRandomCard();
};