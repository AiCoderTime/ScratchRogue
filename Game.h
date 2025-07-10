#pragma once
#include <SFML/Graphics.hpp>
#include "ScratchCard.h"
#include "Player.h"
#include "Shop.h"
#include "ShopView.h"
#include <vector>
#include "ResourceManager.h"  // Add this include

struct Particle {
    sf::Sprite sprite;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float lifetime; // seconds
};

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

    void loadResources();

private:
    void processEvents();
    void update(float dt);
    void render();

    void updateWindowScale();

    void toggleFullscreen();

    void drawOwnedCards(sf::RenderTarget& target);

    void startNewRound();
    void checkRoundEnd();

    void triggerGameOver();

    sf::RenderWindow window;
    sf::RenderTexture virtualCanvas;
    sf::Sprite finalSprite;

    static const int VIRTUAL_WIDTH = 1280;
    static const int VIRTUAL_HEIGHT = 720;

    float windowScale;
    float offsetX, offsetY;

    Player player;
    Shop shop;
    std::unique_ptr<ShopView> shopView;

    GameState currentState = GameState::SHOP;

    sf::Text balanceText;  // Font is now from ResourceManager, no sf::Font member needed
    sf::Text winningsText;

    bool isScratching;

    std::vector<Particle> particles;

    sf::Clock deltaClock;

    std::vector<std::unique_ptr<ScratchCard>> scratchCards;
    std::vector<std::string> ownedCardsToScratch;
    size_t currentCardIndex = 0;


    bool cardProcessed = false;

    // ROUNDS
    int currentRound = 1;
    int quota = 50;
    int roundEarnings = 0;
    bool gameOver = false;

    sf::Text quotaText;
    sf::Text roundEarningsText;


    bool shopActive = false;




};
