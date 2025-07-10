#include "Game.h"
#include "ResourceManager.h"
#include <cmath>
#include <memory>

namespace {
    constexpr unsigned int DEFAULT_WIDTH = 1280;
    constexpr unsigned int DEFAULT_HEIGHT = 720;
}

Game::Game()
    : window(sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT), "Scratch Card Roguelike", sf::Style::Default),
    virtualCanvas(), // virtual resolution
    isScratching(false), currentState(GameState::SHOP)
{
    loadResources(); // ? load textures before using them

    scratchCard = std::make_unique<ScratchCard>("assets/sprites/card_test.png", "assets/sprites/card_test_overlay.png", 2.f);
    shopView = std::make_unique<ShopView>();

    virtualCanvas.create(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    virtualCanvas.setSmooth(false);
    finalSprite.setTexture(virtualCanvas.getTexture());
    finalSprite.setTextureRect(sf::IntRect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT));

    // Center the scratch card
    float x = (DEFAULT_WIDTH - scratchCard->getWidth()) / 2.f;
    float y = (DEFAULT_HEIGHT - scratchCard->getHeight()) / 2.f;
    scratchCard->setPosition(x, y);

    balanceText.setFont(ResourceManager::getFont("mainFont"));
    balanceText.setCharacterSize(24);
    balanceText.setFillColor(sf::Color::White);

    winningsText.setFont(ResourceManager::getFont("mainFont"));
    winningsText.setCharacterSize(24);
    winningsText.setFillColor(sf::Color::Yellow);

    deltaClock.restart();
    updateWindowScale(); // Initial scale
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        float dt = deltaClock.restart().asSeconds();
        update(dt);
        render();
    }
}

void Game::loadResources() {
    // Load fonts
    ResourceManager::loadFont("mainFont", "assets/fonts/retro.ttf");

    // Load textures
    ResourceManager::loadTexture("dust", "assets/sprites/dust.png");
    ResourceManager::loadTexture("card_test", "assets/sprites/card_test.png");
    ResourceManager::loadTexture("card_test_overlay", "assets/sprites/card_test_overlay.png");
    ResourceManager::loadTexture("shop_bg", "assets/sprites/shop_bg.png");

    // Add more resources here if needed
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            updateWindowScale();
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            else if (event.key.code == sf::Keyboard::F11) {
                toggleFullscreen();
            }
            else if (event.key.code == sf::Keyboard::S) {
                shopActive = !shopActive;
            }
        }
        else if (shopActive && event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float x = (mousePos.x - offsetX) / windowScale;
            float y = (mousePos.y - offsetY) / windowScale;
            shopView->handleClick(x, y, player);
        }
        else if (!shopActive && event.type == sf::Event::MouseButtonPressed) {
            isScratching = true;
        }
        else if (!shopActive && event.type == sf::Event::MouseButtonReleased) {
            isScratching = false;
        }
    }
}

void Game::update(float dt) {
    if (isScratching) {
        sf::Vector2i mousePosWindow = sf::Mouse::getPosition(window);
        float virtualMouseX = (mousePosWindow.x - offsetX) / windowScale;
        float virtualMouseY = (mousePosWindow.y - offsetY) / windowScale;

        bool scratched = scratchCard->scratchAt(virtualMouseX, virtualMouseY, player);

        if (scratched) {
            Particle p;
            p.sprite.setTexture(ResourceManager::getTexture("dust"));
            p.sprite.setScale(windowScale, windowScale);
            p.sprite.setPosition(virtualMouseX, virtualMouseY);

            float horizVel = ((rand() % 1000) / 1000.f - 0.5f) * 20.f;
            float vertVel = ((rand() % 500) / 1000.f) * -10.f;

            p.velocity = sf::Vector2f(horizVel, vertVel);
            p.acceleration = sf::Vector2f(0.f, 200.f);
            p.lifetime = 0.5f;

            particles.push_back(p);
        }
    }

    for (auto it = particles.begin(); it != particles.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.f) {
            it = particles.erase(it);
        }
        else {
            it->velocity += it->acceleration * dt;
            it->sprite.move(it->velocity * dt);

            sf::Color c = it->sprite.getColor();
            c.a = static_cast<sf::Uint8>(255 * (it->lifetime / 0.5f));
            it->sprite.setColor(c);

            ++it;
        }
    }
}

void Game::render() {
    if (shopActive) {
        virtualCanvas.clear(sf::Color(30, 30, 30));
        shopView->draw(virtualCanvas);
    }
    else {
        virtualCanvas.clear(sf::Color(50, 50, 50));
        scratchCard->draw(virtualCanvas);
        for (const auto& p : particles) {
            virtualCanvas.draw(p.sprite);
        }
    }
    virtualCanvas.display();

    finalSprite.setTexture(virtualCanvas.getTexture(), true);
    finalSprite.setScale(windowScale, windowScale);
    finalSprite.setPosition(std::round(offsetX), std::round(offsetY));

    window.clear(sf::Color::Black);
    window.draw(finalSprite);

    if (!shopActive) {
        const sf::Font& font = ResourceManager::getFont("mainFont");
        auto prizeTexts = scratchCard->getRevealedPrizeTexts();
        for (const auto& info : prizeTexts) {
            sf::Text text;
            text.setFont(font);
            text.setString(info.text);
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::Black);

            sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);

            float winX = std::round(info.position.x * windowScale + offsetX);
            float winY = std::round(info.position.y * windowScale + offsetY);
            text.setPosition(winX, winY);

            window.draw(text);
        }

        balanceText.setString("Balance: $" + std::to_string(player.getBalance()));
        balanceText.setPosition(10.f, 10.f);
        balanceText.setCharacterSize(40);
        window.draw(balanceText);

        std::string relicsStr;
        for (const auto& relic : player.getRelics()) {
            relicsStr += relic + " ";
        }

        winningsText.setString(
            "Winnings so far: $" + std::to_string(scratchCard->getAccumulatedMoney()) + "\n" +
            "Multiplier: x" + std::to_string(scratchCard->getAccumulatedMultiplier()) + "\n" +
            "Relics: " + relicsStr
        );

        sf::FloatRect balanceBounds = balanceText.getGlobalBounds();
        winningsText.setPosition(10.f, balanceBounds.top + balanceBounds.height + 10.f);

        window.draw(winningsText);

        if (scratchCard->isFullyRevealed()) {
            scratchCard->applyWinningsToPlayer(player);
        }
    }

    window.display();
}

void Game::updateWindowScale() {
    sf::Vector2u realSize = window.getSize();
    float scaleX = static_cast<float>(realSize.x) / DEFAULT_WIDTH;
    float scaleY = static_cast<float>(realSize.y) / DEFAULT_HEIGHT;
    windowScale = std::min(scaleX, scaleY);
    offsetX = (realSize.x - DEFAULT_WIDTH * windowScale) / 2.f;
    offsetY = (realSize.y - DEFAULT_HEIGHT * windowScale) / 2.f;
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
