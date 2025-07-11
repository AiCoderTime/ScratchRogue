#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "ScratchCard.h"
#include "Player.h"
#include "Shop.h"
#include "ShopView.h"
#include "ResourceManager.h"

// Simple particle system for scratch effects
struct Particle {
    sf::Sprite sprite;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float lifetime; // in seconds
};

// Main game states
enum class GameState {
    SHOP,
    SCRATCHING,
    RESULT,
    GAME_OVER
};

class Game {
public:
    Game();
    void run();

private:
    // Core game loop helpers
    void processEvents();
    void update(float dt);
    void render();

    // Window scale and fullscreen handling
    void updateWindowScale();
    void toggleFullscreen();

    // Drawing helpers
    void drawOwnedCards(sf::RenderTarget& target);

    // Round and game state management
    void startNewRound();
    void checkRoundEnd();
    void triggerGameOver();

    // Resource loading helper
    void loadResources();

private:
    sf::RenderWindow window;
    sf::RenderTexture virtualCanvas;  // For scaling and smoothing
    sf::Sprite finalSprite;

    static const int VIRTUAL_WIDTH = 1280;
    static const int VIRTUAL_HEIGHT = 720;

    float windowScale = 1.f;
    float offsetX = 0.f;
    float offsetY = 0.f;

    Player player;
    Shop shop;
    std::unique_ptr<ShopView> shopView;

    GameState currentState = GameState::SHOP;

    // UI texts
    sf::Text balanceText;
    sf::Text winningsText;
    sf::Text quotaText;
    sf::Text roundEarningsText;

    bool isScratching = false;

    std::vector<Particle> particles;

    sf::Clock deltaClock;

    std::vector<std::unique_ptr<ScratchCard>> scratchCards;
    std::vector<std::string> ownedCardsToScratch;
    size_t currentCardIndex = 0;

    bool cardProcessed = false;

    // Round variables
    int currentRound = 1;
    int quota = 50;
    int roundEarnings = 0;
    bool gameOver = false;

    bool shopActive = false;
};
