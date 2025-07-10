#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "ResourceManager.h"
#include "Player.h"

struct ShopItem {
    enum class Type { Card, Relic };
    Type type;
    std::string id; // For cards: card type/id, for relics: relic id
    int price;
    sf::Sprite icon;
};

class ShopView {
public:
    ShopView();
    void reroll();
    void draw(sf::RenderTarget& target);
    void handleClick(float x, float y, Player& player);

private:
    std::vector<ShopItem> items; // 3 cards + 2 relics
    sf::Sprite background;
    sf::Sprite rerollButton;
    sf::Text priceText;
    sf::Font& font;
    sf::Vector2f cardPositions[3];
    sf::Vector2f relicPositions[2];
    sf::Vector2f rerollButtonPos;
    void setupSprites();
    void updateIcons();
};