#pragma once
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "CardTypes.h"
#include "SymbolType.h"

enum class Rarity { Common, Uncommon, Rare, Epic, Legendary };

struct ScratchCardOffer {
	std::string name;
	Rarity rarity;
	std::vector<CardType> cardTypes;
	int cost;
	sf::Sprite previewSprite;
	std::vector<SymbolType> allowedSymbols;
};