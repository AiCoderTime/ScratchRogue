#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "CardTypes.h"
#include "ResourceManager.h"
#include "Player.h"

// Represents an item in the shop (either card or relic)
struct ShopItem {
    enum class Type { Card, Relic } type;
    std::string id;         // Unique ID string for resource lookup
    int price;              // Price in game currency
    sf::Sprite icon;        // Sprite used to render the item

    // Only relevant for cards
    std::string rarity;     // e.g. "Common", "Uncommon", "Rare"
    CardType cardType1;     // Primary card type
    CardType cardType2;     // Secondary card type
};

class ShopView {
public:
    ShopView();

    // Generate new shop items (cards + relics)
    void reroll();

    // Draw the shop UI (background, items, buttons, prices)
    void draw(sf::RenderTarget& target);

    // Handle mouse click at (x,y), purchase items, or activate buttons
    void handleClick(float x, float y, Player& player);

    int getCardsBought() const { return cardsBought; }
    void resetCardsBought() { cardsBought = 0; }

    // Callback for when next round button is clicked
    std::function<void()> onNextRoundClicked;

private:
    sf::Font font;
    sf::Text priceText;
    sf::Sprite background;
    sf::Sprite rerollButton;
    sf::Sprite nextRoundButton;

    static constexpr float GAME_PIXEL_SCALE = 3.f;

    sf::Vector2f cardPositions[3];   // Positions for 3 card slots
    sf::Vector2f relicPositions[2];  // Positions for 2 relic slots
    sf::Vector2f rerollButtonPos;
    sf::Vector2f nextRoundButtonPos;

    std::vector<ShopItem> items;     // Currently displayed shop items

    int cardsBought = 0;

    // Helpers (implementations omitted here)
    int getPriceForRarity(const std::string& rarity);
};
