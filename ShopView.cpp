#include "ShopView.h"
#include "Player.h"
#include <random>

constexpr float GAME_PIXEL_SCALE = 3.f;

ShopView::ShopView() : font(ResourceManager::getFont("mainFont")) {
    background.setTexture(ResourceManager::getTexture("shop_bg"));
    // Center background sprite (128x128 scaled by 3)
    sf::Vector2u textureSize = background.getTexture()->getSize(); // 128x128
    const float screenWidth = 1280.f;
    const float screenHeight = 720.f;
    background.setPosition(
        (screenWidth - textureSize.x * GAME_PIXEL_SCALE) / 2.f,
        (screenHeight - textureSize.y * GAME_PIXEL_SCALE) / 2.f
    );
    background.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

    sf::Vector2f bgPos = background.getPosition();
    sf::Vector2f bgScale = background.getScale();

    // Positions relative to background (adjust as needed)
    const sf::Vector2f cardLocalCenters[3] = {
        {27.f, 31.f},   // left card center
        {75.f, 31.f},   // center card center
        {123.f, 31.f}   // right card center
    };

    const sf::Vector2f relicLocalCenters[2] = {
        {60.f, 90.f},   // left relic center
        {90.f, 90.f}    // right relic center
    };

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

    rerollButtonPos = sf::Vector2f(
        bgPos.x + 96.f * bgScale.x,
        bgPos.y + 110.f * bgScale.y
    );

    rerollButton.setTexture(ResourceManager::getTexture("reroll_button"));
    rerollButton.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

    // Center reroll button at bottom with padding
    float buttonWidth = rerollButton.getTexture()->getSize().x * GAME_PIXEL_SCALE;
    float buttonHeight = rerollButton.getTexture()->getSize().y * GAME_PIXEL_SCALE;
    rerollButtonPos = sf::Vector2f(
        (screenWidth - buttonWidth) / 2.f,
        screenHeight - buttonHeight - 200.f
    );
    rerollButton.setPosition(rerollButtonPos);

    nextRoundButton.setTexture(ResourceManager::getTexture("next_round_button"));
    nextRoundButton.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

    nextRoundButtonPos = sf::Vector2f(
        rerollButtonPos.x,
        rerollButtonPos.y + rerollButton.getTexture()->getSize().y * GAME_PIXEL_SCALE + 10.f
    );
    nextRoundButton.setPosition(nextRoundButtonPos);

    priceText.setFont(font);
    priceText.setCharacterSize(static_cast<unsigned int>(12 * GAME_PIXEL_SCALE));
    priceText.setFillColor(sf::Color::White);

    reroll();
}

void ShopView::reroll() {
    items.clear();
    // Generate 3 cards - all "lucky_7_shop" icons in shop
    for (int i = 0; i < 3; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Card;

        // Card ID is shop preview version
        item.id = "lucky_7_shop"; // shop preview ID

        item.rarity = "Common";
        item.price = getPriceForRarity(item.rarity);

        // Use shop preview texture (small icon)
        item.icon.setTexture(ResourceManager::getTexture(item.id));
        sf::Vector2u texSize = item.icon.getTexture()->getSize();
        item.icon.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        item.icon.setPosition(cardPositions[i]);
        item.icon.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);

        items.push_back(item);
    }
    cardsBought = 0; // reset on reroll

    // Generate 2 relics randomly
    for (int i = 0; i < 2; ++i) {
        ShopItem item;
        item.type = ShopItem::Type::Relic;
        item.id = "relic_" + std::to_string(rand() % 5);
        item.price = 20 + rand() % 10;

        // Relic icon texture always uses relic_1, relic_2, etc.
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
        priceText.setString("£" + std::to_string(item.price));
        sf::FloatRect bounds = priceText.getLocalBounds();
        priceText.setOrigin(bounds.width / 2.f, 0.f);

        float yOffset;
        if (item.type == ShopItem::Type::Card) {
            yOffset = 25.f * GAME_PIXEL_SCALE;   // card price offset (lower)
        }
        else if (item.type == ShopItem::Type::Relic) {
            yOffset = 10.f * GAME_PIXEL_SCALE;    // relic price offset (higher)
        }
        else {
            yOffset = 25.f * GAME_PIXEL_SCALE;   // default fallback
        }

        priceText.setPosition(item.icon.getPosition().x, item.icon.getPosition().y + yOffset);
        target.draw(priceText);
    }
    target.draw(rerollButton);
    target.draw(nextRoundButton);
}

void ShopView::handleClick(float x, float y, Player& player) {
    if (rerollButton.getGlobalBounds().contains(x, y)) {
        reroll();
        return;
    }

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
                    // Pass the shop preview ID (ScratchCard will remap internally)
                    player.addCard(it->id);
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

    if (nextRoundButton.getGlobalBounds().contains(x, y)) {
        if (onNextRoundClicked) {
            onNextRoundClicked();
        }
        return;
    }
}

int ShopView::getPriceForRarity(const std::string& rarity) {
    if (rarity == "Common") return 3;
    else if (rarity == "Uncommon") return 5;
    else if (rarity == "Rare") return 10;
    return 3;
}
