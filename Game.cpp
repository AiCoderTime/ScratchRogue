#include "Game.h"
#include "ResourceManager.h"
#include <cmath>
#include <memory>
#include <string>
#include <iostream>

namespace {
    constexpr unsigned int DEFAULT_WIDTH = 1280;
    constexpr unsigned int DEFAULT_HEIGHT = 720;
    constexpr float GAME_PIXEL_SCALE = 3.f;
}

Game::Game()
    : window(sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT), "Scratch Card Roguelike", sf::Style::Default),
    isScratching(false),
    currentState(GameState::SHOP),
    windowScale(1.f),
    offsetX(0.f),
    offsetY(0.f),
    currentCardIndex(0),
    cardProcessed(false)
{
    loadResources();

    shopView = std::make_unique<ShopView>();

    shopView->onNextRoundClicked = [this]() {
        const auto& ownedCardCounts = player.getOwnedCardsCount();
        if (!ownedCardCounts.empty()) {
            // Initialize round variables here:
            roundEarnings = 0;
            quota = 50 + (currentRound - 1) * 20;
            gameOver = false;

            // Build list of cards to scratch (each repeated by count)
            ownedCardsToScratch.clear();
            for (const auto& [cardId, count] : ownedCardCounts) {
                for (int i = 0; i < count; ++i) {
                    ownedCardsToScratch.push_back(cardId);
                }
            }
            currentCardIndex = 0;
            cardProcessed = false;

            // DEBUG: Print owned cards to scratch
            std::cout << "[Debug] Owned cards to scratch (" << ownedCardsToScratch.size() << "): ";
            for (const auto& c : ownedCardsToScratch) {
                std::cout << c << " ";
            }
            std::cout << std::endl;

            // Create ScratchCard instances for all cards
            scratchCards.clear();
            for (const auto& cardId : ownedCardsToScratch) {
                auto sc = std::make_unique<ScratchCard>("assets/sprites/lucky_7.png", "assets/sprites/lucky_7_overlay.png", GAME_PIXEL_SCALE);
                sc->loadCard(cardId);
                sc->resetScratch();

                sc->setPosition(
                    (DEFAULT_WIDTH - sc->getWidth()) / 2.f,
                    (DEFAULT_HEIGHT - sc->getHeight()) / 2.f
                );

                scratchCards.push_back(std::move(sc));
            }

            if (!scratchCards.empty()) {
                currentState = GameState::SCRATCHING;
                std::cout << "Starting scratch round with " << scratchCards.size() << " cards.\n";
            }
        }
        else {
            std::cout << "You must buy at least one card before starting the round!\n";
        }
        };



    virtualCanvas.create(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    virtualCanvas.setSmooth(false);

    finalSprite.setTexture(virtualCanvas.getTexture());
    finalSprite.setTextureRect({ 0, 0, static_cast<int>(DEFAULT_WIDTH), static_cast<int>(DEFAULT_HEIGHT) });

    // Setup text objects
    const auto& mainFont = ResourceManager::getFont("mainFont");
    balanceText.setFont(mainFont);
    winningsText.setFont(mainFont);

    quotaText.setFont(ResourceManager::getFont("mainFont"));
    quotaText.setCharacterSize(static_cast<unsigned int>(12 * GAME_PIXEL_SCALE));
    quotaText.setFillColor(sf::Color::Cyan);  // Or another color you like

    

    roundEarningsText.setFont(ResourceManager::getFont("mainFont"));
	roundEarningsText.setCharacterSize(static_cast<unsigned int>(12 * GAME_PIXEL_SCALE));
	roundEarningsText.setFillColor(sf::Color::Yellow);


    for (auto* textObj : { &balanceText, &winningsText }) {
        textObj->setCharacterSize(static_cast<unsigned int>(12 * GAME_PIXEL_SCALE));
        textObj->setFillColor(textObj == &balanceText ? sf::Color::White : sf::Color::Yellow);
    }

    deltaClock.restart();
    updateWindowScale();
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update(deltaClock.restart().asSeconds());
        render();
    }
}

void Game::loadResources() {
    ResourceManager::loadFont("mainFont", "assets/fonts/retro.ttf");

    // Textures for effects, UI, and cards
    ResourceManager::loadTexture("dust", "assets/sprites/dust.png");
    ResourceManager::loadTexture("shop_bg", "assets/sprites/shop_bg.png");
    ResourceManager::loadTexture("reroll_button", "assets/sprites/reroll_button.png");
    ResourceManager::loadTexture("next_round_button", "assets/sprites/next_round_button.png");
    ResourceManager::loadTexture("relic_1", "assets/sprites/relic_1.png");
    ResourceManager::loadTexture("relic_2", "assets/sprites/relic_2.png");
    ResourceManager::loadTexture("card_shop", "assets/sprites/card_shop.png");

    // Card related
    ResourceManager::loadTexture("lucky_7", "assets/sprites/lucky_7.png");
    ResourceManager::loadTexture("lucky_7_shop", "assets/sprites/lucky_7_shop.png");  // small shop sprite

    // Symbols
    ResourceManager::loadTexture("empty", "assets/sprites/symbols/empty.png");
    ResourceManager::loadTexture("7", "assets/sprites/symbols/7.png");
    // Add more textures here as needed
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            window.close();
            break;

        case sf::Event::Resized:
            updateWindowScale();
            break;

        case sf::Event::KeyPressed:
            switch (event.key.code) {
            case sf::Keyboard::Escape:
                window.close();
                break;
            case sf::Keyboard::F11:
                toggleFullscreen();
                break;
            case sf::Keyboard::M:
                player.addBalance(10);
                std::cout << "[Debug] Added £10 to player balance. New balance: £" << player.getBalance() << "\n";
                break;
            case sf::Keyboard::A:
                if (currentState == GameState::SCRATCHING && !scratchCards.empty()) {
                    scratchCards[currentCardIndex]->startAutoScratch();
                    std::cout << "Auto scratch started for current card.\n";
                }
                break;
            default:
                break;
            }
            break;

        case sf::Event::MouseButtonPressed:
            if (currentState == GameState::SHOP) {
                auto mousePos = sf::Mouse::getPosition(window);
                float x = (mousePos.x - offsetX) / windowScale;
                float y = (mousePos.y - offsetY) / windowScale;
                shopView->handleClick(x, y, player);
            }
            else if (currentState == GameState::SCRATCHING) {
                isScratching = true;
            }
            break;

        case sf::Event::MouseButtonReleased:
            if (currentState == GameState::SCRATCHING) {
                isScratching = false;
            }
            break;

        default:
            break;
        }
    }
}

void Game::update(float dt) {
    if (currentState == GameState::SCRATCHING && isScratching) {
        auto mousePos = sf::Mouse::getPosition(window);
        float virtualX = (mousePos.x - offsetX) / windowScale;
        float virtualY = (mousePos.y - offsetY) / windowScale;

        if (!scratchCards.empty() && scratchCards[currentCardIndex]->scratchAt(virtualX, virtualY, player)) {
            Particle p;
            p.sprite.setTexture(ResourceManager::getTexture("dust"));
            p.sprite.setScale(GAME_PIXEL_SCALE * windowScale, GAME_PIXEL_SCALE * windowScale);
            p.sprite.setPosition(virtualX, virtualY);

            p.velocity = sf::Vector2f(((rand() % 1000) / 1000.f - 0.5f) * 20.f, ((rand() % 500) / 1000.f) * -10.f);
            p.acceleration = sf::Vector2f(0.f, 200.f);
            p.lifetime = 0.5f;

            particles.push_back(std::move(p));
        }
    }

    // Update particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.f) {
            it = particles.erase(it);
        }
        else {
            it->velocity += it->acceleration * dt;
            it->sprite.move(it->velocity * dt);

            sf::Color color = it->sprite.getColor();
            color.a = static_cast<sf::Uint8>(255 * (it->lifetime / 0.5f));
            it->sprite.setColor(color);

            ++it;
        }
    }

    // Handle card reveal & progression
    if (currentState == GameState::SCRATCHING) {
        if (scratchCards.empty()) return;

        // If auto scratch active on current card, update it
        scratchCards[currentCardIndex]->updateAutoScratch(dt, player);

        // Only allow manual scratching if autoScratchActive is false
        if (!scratchCards[currentCardIndex]->autoScratchActive && isScratching) {
            // Existing manual scratch code here...
        }

        bool fullyRevealed = scratchCards[currentCardIndex]->isFullyRevealed();

        if (fullyRevealed && !cardProcessed) {
            cardProcessed = true;

            std::cout << "ScratchCard is fully revealed now!\n";

            scratchCards[currentCardIndex]->applyWinningsToPlayer(player);

            int prizeValue = scratchCards[currentCardIndex]->getAccumulatedMoney();
            roundEarnings += prizeValue;
            std::cout << "Added £" << prizeValue << " to round earnings (Total: £" << roundEarnings << ")\n";


            if (!ownedCardsToScratch.empty() && currentCardIndex < ownedCardsToScratch.size()) {
                player.useCard(ownedCardsToScratch[currentCardIndex]);
                std::cout << "Used card " << ownedCardsToScratch[currentCardIndex] << ".\n";
            }
            else {
                std::cout << "[Warning] Tried to use card at index " << currentCardIndex << " but out of range.\n";
            }

            // Move to next card
            currentCardIndex++;

            // Check if round ended
            if (currentCardIndex >= scratchCards.size()) {
                if (roundEarnings < quota) {
                    triggerGameOver();
                }
                else {
                    // Passed quota, next round setup
                    currentRound++;
                    currentState = GameState::SHOP;
                    shopActive = true;
                    shopView->reroll();
                    std::cout << "Round complete! Moving to shop for round " << currentRound << ".\n";
                }
            }
            else {
                // Load next card
                scratchCards[currentCardIndex]->resetScratch();
                scratchCards[currentCardIndex]->setPosition(
                    (DEFAULT_WIDTH - scratchCards[currentCardIndex]->getWidth()) / 2.f,
                    (DEFAULT_HEIGHT - scratchCards[currentCardIndex]->getHeight()) / 2.f
                );
                cardProcessed = false;  // reset for next card
                std::cout << "Loaded next card (" << ownedCardsToScratch[currentCardIndex] << ") for scratching.\n";
            }


        }
        else if (!fullyRevealed) {
            // Reset flag if card no longer fully revealed (e.g. new card)
            cardProcessed = false;
        }
    }
}

void Game::render() {
    // Clear virtual canvas based on current state
    virtualCanvas.clear(currentState == GameState::SHOP ? sf::Color(30, 30, 30) : sf::Color(50, 50, 50));

    if (currentState == GameState::SHOP) {
        particles.clear();
        shopView->draw(virtualCanvas);

        // Draw owned cards preview on the right side in SHOP as well
        drawOwnedCards(virtualCanvas);
    }
    else if (currentState == GameState::SCRATCHING) {
        if (!scratchCards.empty()) {
            // Center the card before drawing each frame
            auto& sc = scratchCards[currentCardIndex];
            sc->setPosition(
                (DEFAULT_WIDTH - sc->getWidth()) / 2.f,
                (DEFAULT_HEIGHT - sc->getHeight()) / 2.f
            );

            sc->drawBase(virtualCanvas);
            sc->drawPrizes(virtualCanvas);
        


            // Draw multiplier prize texts
            const sf::Font& font = ResourceManager::getFont("mainFont");
            for (const auto& info : scratchCards[currentCardIndex]->getRevealedPrizeTexts()) {
                sf::Text text(info.text, font, static_cast<unsigned int>(6 * GAME_PIXEL_SCALE));
                text.setFillColor(sf::Color::Black);

                sf::FloatRect bounds = text.getLocalBounds();
                text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
                text.setPosition(info.position);

                virtualCanvas.draw(text);
            }

            scratchCards[currentCardIndex]->drawOverlay(virtualCanvas);
        }

        // Draw particles
        for (const auto& p : particles) {
            virtualCanvas.draw(p.sprite);
        }

        // Draw owned cards preview on the right side in SCRATCHING
        drawOwnedCards(virtualCanvas);
    }

    if (gameOver) {
        sf::Text gameOverText;
        gameOverText.setFont(ResourceManager::getFont("mainFont"));
        gameOverText.setString("Game Over!\nYou failed to reach quota of £" + std::to_string(quota));
        gameOverText.setCharacterSize(static_cast<unsigned int>(6 * GAME_PIXEL_SCALE));
        gameOverText.setFillColor(sf::Color::Red);

        sf::FloatRect bounds = gameOverText.getLocalBounds();
        gameOverText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        gameOverText.setPosition(DEFAULT_WIDTH / 2.f, DEFAULT_HEIGHT / 2.f);

        virtualCanvas.draw(gameOverText);
    }


    virtualCanvas.display();

    finalSprite.setTexture(virtualCanvas.getTexture(), true);
    finalSprite.setScale(windowScale, windowScale);
    finalSprite.setPosition(std::round(offsetX), std::round(offsetY));

    window.clear(sf::Color::Black);
    window.draw(finalSprite);

    // Draw balance text always at top-left for all states
    balanceText.setString("Balance: £" + std::to_string(player.getBalance()));
    balanceText.setPosition(10.f, 10.f);
    window.draw(balanceText);

    quotaText.setString("Quota: £" + std::to_string(quota));
    quotaText.setPosition(515.f, 600.f);  // Just below balanceText, adjust as needed
    window.draw(quotaText);

    roundEarningsText.setString("Round Earnings: £" + std::to_string(roundEarnings));
    roundEarningsText.setPosition(415.f, 635.f);  // Just below balanceText, adjust as needed
    window.draw(roundEarningsText);


    if (currentState == GameState::SCRATCHING && !scratchCards.empty()) {
        std::string relicsStr;
        for (const auto& relic : player.getRelics()) {
            relicsStr += relic + " ";
        }

        winningsText.setString(
            "Winnings so far: £" + std::to_string(scratchCards[currentCardIndex]->getAccumulatedMoney()) + "\n" +
            "Multiplier: x" + std::to_string(scratchCards[currentCardIndex]->getAccumulatedMultiplier()) + "\n" +
            "Relics: " + relicsStr
        );

        sf::FloatRect balanceBounds = balanceText.getGlobalBounds();
        winningsText.setPosition(10.f, balanceBounds.top + balanceBounds.height + 10.f);
        window.draw(winningsText);

    }

    window.display();
}

void Game::updateWindowScale() {
    sf::Vector2u size = window.getSize();
    float scaleX = static_cast<float>(size.x) / DEFAULT_WIDTH;
    float scaleY = static_cast<float>(size.y) / DEFAULT_HEIGHT;

    windowScale = std::min(scaleX, scaleY);
    offsetX = (size.x - DEFAULT_WIDTH * windowScale) / 2.f;
    offsetY = (size.y - DEFAULT_HEIGHT * windowScale) / 2.f;
}

void Game::toggleFullscreen() {
    static bool isFullscreen = false;
    isFullscreen = !isFullscreen;

    window.create(
        isFullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT),
        "Scratch Card Roguelike",
        isFullscreen ? sf::Style::Fullscreen : sf::Style::Default
    );
    updateWindowScale();
}

void Game::drawOwnedCards(sf::RenderTarget& target) {
    // Use the *small* shop preview texture ID here for the preview and count
    const sf::Texture& cardPreviewTexture = ResourceManager::getTexture("lucky_7_shop");

    int ownedLucky7Count = 0;
    const auto& ownedCardsCount = player.getOwnedCardsCount();
    auto it = ownedCardsCount.find("lucky_7_shop");
    if (it != ownedCardsCount.end()) {
        ownedLucky7Count = it->second;
    }

    if (ownedLucky7Count > 0) {
        float x = DEFAULT_WIDTH - 250.f;  // Adjust horizontal position as needed
        float y = 50.f;

        sf::Sprite cardSprite(cardPreviewTexture);
        cardSprite.setScale(GAME_PIXEL_SCALE, GAME_PIXEL_SCALE);
        cardSprite.setPosition(x, y);
        target.draw(cardSprite);

        sf::Text countText;
        countText.setFont(ResourceManager::getFont("mainFont"));
        countText.setCharacterSize(static_cast<unsigned int>(14 * GAME_PIXEL_SCALE));
        countText.setFillColor(sf::Color::White);
        countText.setString("x" + std::to_string(ownedLucky7Count));

        sf::FloatRect spriteBounds = cardSprite.getGlobalBounds();
        countText.setPosition(spriteBounds.left + spriteBounds.width + 5.f, y + spriteBounds.height / 4.f);
        target.draw(countText);
    }
}

// ROUND SYSTEM

void Game::checkRoundEnd() {
    // If all cards scratched
    if (currentCardIndex >= scratchCards.size()) {
        if (roundEarnings < quota) {
            triggerGameOver();
        }
        else {
            // Passed quota, next round
            currentRound++;
            currentState = GameState::SHOP;
            shopActive = true;
            shopView->reroll();
        }
    }
}

void Game::triggerGameOver() {
    gameOver = true;
    currentState = GameState::RESULT; // You might want to create a GAME_OVER state or use RESULT with a special message
    std::cout << "Game Over! You failed to reach quota of £" << quota << "!\n";
    // You can add more game-over handling here, e.g., show message, reset game, etc.
}

