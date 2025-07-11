#include "ShopView.h"
#include "Player.h"
#include <random>

// Constants
constexpr float GAME_PIXEL_SCALE = 3.f;
constexpr float SCREEN_WIDTH = 1280.f;
constexpr float SCREEN_HEIGHT = 720.f;

ShopView::ShopView() : font(ResourceManager::getFont("mainFont")) {
    // Setup background
    background.setTexture(ResourceManager::getTexture("shop_bg"));
    sf::Vector2u texSize = background.getTexture()->getSize(); // e.g. 128x128
    background.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);
    background.setPosition(
        (SCREEN_WIDTH - texSize.x * GAME_PIXEL_SCALE) / 2.f,
        (SCREEN_HEIGHT - texSize.y * GAME_PIXEL_SCALE) / 2.f
    );

    sf::Vector2f bgPos = background.getPosition();
    sf::Vector2f bgScale = background.getScale();

    // Relative centers for card icons inside the background
    const sf::Vector2f cardLocalCenters[3] = {
        {27.f, 31.f}, {75.f, 31.f}, {123.f, 31.f}
    };

    // Relative centers for relic icons inside the background
    const sf::Vector2f relicLocalCenters[2] = {
        {60.f, 90.f}, {90.f, 90.f}
    };

    // Calculate absolute positions for cards and relics
    for (int i = 0; i < 3; ++i) {
        cardPositions[i] = sf::Vector2f(
            bgPos.x + cardLocalCenters[i].x * bgScale.x,
            bgPos.y + cardLocalCenters[i].y * bgScale.y
        );
    }
    for (int i = 0; i < 2; ++i) {
        relicPositions[i] = sf::Vector2f(
            bgPos.x + relicLocalCenters[i].x * bgScale.x,
            bgPos.y + relicLocalCenters[i].y * bgScale.y
        );
    }

    // Setup reroll button near bottom center
    rerollButton.setTexture(ResourceManager::getTexture("reroll_button"));
    rerollButton.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

    float buttonWidth = rerollButton.getTexture()->getSize().x * GAME_PIXEL_SCALE;
    float buttonHeight = rerollButton.getTexture()->getSize().y * GAME_PIXEL_SCALE;

    rerollButtonPos = sf::Vector2f(
        (SCREEN_WIDTH - buttonWidth) / 2.f,
        SCREEN_HEIGHT - buttonHeight - 200.f
    );
    rerollButton.setPosition(rerollButtonPos);

    // Setup next round button below reroll button
    nextRoundButton.setTexture(ResourceManager::getTexture("next_round_button"));
    nextRoundButton.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

    nextRoundButtonPos = sf::Vector2f(
        rerollButtonPos.x,
        rerollButtonPos.y + buttonHeight + 10.f
    );
    nextRoundButton.setPosition(nextRoundButtonPos);

    // Setup price text style
    priceText.setFont(font);
    priceText.setCharacterSize(static_cast<unsigned int>(12 * GAME_PIXEL_SCALE));
    priceText.setFillColor(sf::Color::White);

    reroll(); // Initialize shop items
}

void ShopView::reroll() {
    items.clear();

    // Always generate 3 cards - currently all with preview "lucky_7_shop"
    for (int i = 0; i < 3; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Card;
        item.id = "lucky_7_shop";  // Shop preview texture ID
        item.rarity = "Common";
        item.price = getPriceForRarity(item.rarity);

        // Setup sprite icon for the shop preview
        item.icon.setTexture(ResourceManager::getTexture(item.id));
        sf::Vector2u texSize = item.icon.getTexture()->getSize();
        item.icon.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        item.icon.setPosition(cardPositions[i]);
        item.icon.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

        items.push_back(item);
    }

    cardsBought = 0; // Reset purchase count on reroll

    // Generate 2 relics randomly
    for (int i = 0; i < 2; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Relic;
        item.id = "relic_" + std::to_string(rand() % 5); // random relic id from relic_0 to relic_4
        item.price = 20 + rand() % 10;                   // price between 20 and 29

        // Use fixed relic texture slots (relic_1, relic_2)
        item.icon.setTexture(ResourceManager::getTexture("relic_" + std::to_string(i + 1)));
        sf::Vector2u texSize = item.icon.getTexture()->getSize();
        item.icon.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        item.icon.setPosition(relicPositions[i]);
        item.icon.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

        items.push_back(item);
    }
}

void ShopView::draw(sf::RenderTarget& target) {
    target.draw(background);

    for (const auto& item : items) {
        target.draw(item.icon);

        // Set price text string and center it horizontally under the icon
        priceText.setString("£" + std::to_string(item.price));
        sf::FloatRect bounds = priceText.getLocalBounds();
        priceText.setOrigin(bounds.width / 2.f, 0.f);

        float yOffset = (item.type == ShopItem::Type::Relic) ? 10.f * GAME_PIXEL_SCALE : 25.f * GAME_PIXEL_SCALE;
        priceText.setPosition(item.icon.getPosition().x, item.icon.getPosition().y + yOffset);
        target.draw(priceText);
    }

    target.draw(rerollButton);
    target.draw(nextRoundButton);
}

void ShopView::handleClick(float x, float y, Player& player) {
    // Check reroll button first
    if (rerollButton.getGlobalBounds().contains(x, y)) {
        reroll();
        return;
    }

    // Check clicks on items (cards or relics)
    for (auto it = items.begin(); it != items.end(); /* no increment here */) {
        if (it->icon.getGlobalBounds().contains(x, y)) {
            if (it->type == ShopItem::Type::Card) {
                if (cardsBought >= 3) {
                    std::cout << "You can only buy 3 cards before rerolling.\n";
                    return;
                }
                if (player.getBalance() >= it->price) {
                    player.addBalance(-it->price);
                    cardsBought++;
                    player.addCard(it->id); // Add card by its shop preview ID
                    std::cout << "Bought card: " << it->id << " for £" << it->price << "\n";
                    it = items.erase(it);
                    return;
                }
                else {
                    std::cout << "Not enough money to buy card.\n";
                    return;
                }
            }
            else if (it->type == ShopItem::Type::Relic) {
                if (player.getBalance() >= it->price) {
                    player.addBalance(-it->price);
                    player.addRelic(it->id);
                    std::cout << "Bought relic: " << it->id << " for £" << it->price << "\n";
                    it = items.erase(it);
                    return;
                }
                else {
                    std::cout << "Not enough money to buy relic.\n";
                    return;
                }
            }
        }
        else {
            ++it;
        }
    }

    // Check next round button click
    if (nextRoundButton.getGlobalBounds().contains(x, y)) {
        if (onNextRoundClicked) {
            onNextRoundClicked();
        }
    }
}

int ShopView::getPriceForRarity(const std::string& rarity) {
    if (rarity == "Common") return 3;
    if (rarity == "Uncommon") return 5;
    if (rarity == "Rare") return 10;
    return 3; // Default fallback price
}
