#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include "Prize.h"

class Player;

struct PrizeTextInfo {
    std::string text;
    sf::Vector2f position;
};

class ScratchCard {
public:
    ScratchCard(const std::string& cardPath, const std::string& overlayPath, float scale);

    bool scratchAt(float x, float y, Player& player);
    void draw(sf::RenderTarget& target) const;

    void setPosition(float x, float y);
    float getWidth() const;
    float getHeight() const;



    std::vector<PrizeTextInfo> getRevealedPrizeTexts() const;

    float getScratchCompletionPercent() const;
    void revealAll();
    bool isFullyRevealed() const;

    void initializeZonesFromOverlay();
    void assignRandomPrizes();

    bool isFullyScratched() const;

    int getAccumulatedMoney() const { return accumulatedMoney; }
    float getAccumulatedMultiplier() const { return accumulatedMultiplier; }
    void applyWinningsToPlayer(Player& player);

    std::string getPrizeText(const Prize& prize) const;

    bool areWinningsApplied() const { return winningsApplied; }

private:
    sf::Texture cardTexture;
    sf::Sprite cardSprite;

    sf::Image overlayImage;
    sf::Texture overlayTexture;
    sf::Sprite overlaySprite;

    sf::Image scratchMask;

    float scale;
    bool fullyRevealed = false;

    void updateOverlayTexture();

    struct Zone {
        sf::IntRect rect;
        int totalPixels = 0;
        int clearedPixels = 0;
        bool revealed = false;
        bool applied = false;
        Prize prize;
    };

    std::vector<Zone> zones;

    void detectZones();
    sf::IntRect floodFill(unsigned int x, unsigned int y, std::vector<std::vector<bool>>& visited);
    void updateZoneClearedPixels(Zone& zone);
    void revealZone(Zone& zone, Player& player);

    int accumulatedMoney = 0;
    float accumulatedMultiplier = 1.f;
    bool winningsApplied = false;
};
