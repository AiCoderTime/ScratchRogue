#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "CardTypes.h"
#include "ResourceManager.h"
#include "Player.h"
#include <functional>

struct ShopItem {
	enum class Type { Card, Relic } type;
	std::string id;
	int price;
	sf::Sprite icon;

	// New for cards:
	std::string rarity; // "Common", "Uncommon", "Rare", etc
	CardType cardType1;
	CardType cardType2;
};

class ShopView {
public:
	ShopView();

	void reroll();
	void draw(sf::RenderTarget& target);
	void handleClick(float x, float y, Player& player);

	int getCardsBought() const { return cardsBought; }
	void resetCardsBought() { cardsBought = 0; }

	std::function<void()> onNextRoundClicked;

private:
	sf::Font font;
	sf::Text priceText;
	sf::Sprite background;
	sf::Sprite rerollButton;

	static constexpr float GAME_PIXEL_SCALE = 3.f;

	sf::Vector2f cardPositions[3];
	sf::Vector2f relicPositions[2];
	sf::Vector2f rerollButtonPos;

	std::vector<ShopItem> items;

	// Helpers
	std::string pickRarity();
	std::pair<CardType, CardType> pickTwoTypesForRarity(const std::string& rarity);
	int getPriceForRarity(const std::string& rarity);

	int cardsBought = 0;

	sf::Sprite nextRoundButton;
	sf::Vector2f nextRoundButtonPos;

};
