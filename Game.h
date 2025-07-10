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
    RESULT
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

    sf::RenderWindow window;
    sf::RenderTexture virtualCanvas;
    sf::Sprite finalSprite;

    static const int VIRTUAL_WIDTH = 1280;
    static const int VIRTUAL_HEIGHT = 720;

    float windowScale;
    float offsetX, offsetY;

    Player player;
    Shop shop;
    GameState currentState = GameState::SHOP;

    sf::Text balanceText;  // Font is now from ResourceManager, no sf::Font member needed
    sf::Text winningsText;

    bool isScratching;

    std::vector<Particle> particles;

    sf::Clock deltaClock;

    std::unique_ptr<ScratchCard> scratchCard;
    std::unique_ptr<ShopView> shopView;

    bool shopActive = false;

};
