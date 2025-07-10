#include "ShopView.h"
#include "Player.h"
#include <random>

ShopView::ShopView() : font(ResourceManager::getFont("mainFont")) {
    background.setTexture(ResourceManager::getTexture("shop_bg"));
    rerollButton.setTexture(ResourceManager::getTexture("reroll_button"));
    rerollButtonPos = sf::Vector2f(1000, 600);
    rerollButton.setPosition(rerollButtonPos);

    // Layout positions for cards and relics
    cardPositions[0] = sf::Vector2f(200, 200);
    cardPositions[1] = sf::Vector2f(500, 200);
    cardPositions[2] = sf::Vector2f(800, 200);
    relicPositions[0] = sf::Vector2f(350, 500);
    relicPositions[1] = sf::Vector2f(700, 500);

    priceText.setFont(font);
    priceText.setCharacterSize(24);
    priceText.setFillColor(sf::Color::White);

    reroll();
}

void ShopView::reroll() {
    items.clear();
    // Randomly generate 3 cards
    for (int i = 0; i < 3; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Card;
        item.id = "card_" + std::to_string(rand() % 5); // Example card id
        item.price = 10 + rand() % 10;
        item.icon.setTexture(ResourceManager::getTexture("card_shop"));
        item.icon.setPosition(cardPositions[i]);
        items.push_back(item);
    }
    // Randomly generate 2 relics
    for (int i = 0; i < 2; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Relic;
        item.id = "relic_" + std::to_string(rand() % 5);
        item.price = 20 + rand() % 10;
        item.icon.setTexture(ResourceManager::getTexture("relic_" + std::to_string(i + 1)));
        item.icon.setPosition(relicPositions[i]);
        items.push_back(item);
    }
}

void ShopView::draw(sf::RenderTarget& target) {
    target.draw(background);
    for (const auto& item : items) {
        target.draw(item.icon);
        priceText.setString("$" + std::to_string(item.price));
        priceText.setPosition(item.icon.getPosition().x, item.icon.getPosition().y + 120);
        target.draw(priceText);
    }
    target.draw(rerollButton);
}

void ShopView::handleClick(float x, float y, Player& player) {
    if (rerollButton.getGlobalBounds().contains(x, y)) {
        reroll();
        return;
    }
    for (auto& item : items) {
        if (item.icon.getGlobalBounds().contains(x, y)) {
            if (player.getBalance() >= item.price) {
                player.addBalance(-item.price); // Subtract price
                if (item.type == ShopItem::Type::Relic) {
                    player.addRelic(item.id);
                }
                // For cards, add to player's deck/inventory as needed
            }
        }
    }
}
