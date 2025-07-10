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

    // Split draw into two methods
    void drawBase(sf::RenderTarget& target) const;
    void drawOverlay(sf::RenderTarget& target) const;

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

    void drawPrizes(sf::RenderTarget& target) const;  // Declare drawPrizes
    std::string getPrizeText(const Prize& prize) const;

    bool areWinningsApplied() const { return winningsApplied; }

    void loadCard(const std::string& cardId);
    void resetScratch();

    // Auto scratch control
    bool autoScratchActive = false;
    size_t autoScratchZoneIndex = 0;
    float autoScratchTimer = 0.f;
    float autoScratchInterval = 0.5f;  // initial delay between zones (seconds)
    float autoScratchAcceleration = 0.9f;  // multiplier to ramp speed up (reduce interval)
    float autoScratchMinInterval = 0.05f;  // minimum interval between zones


    void startAutoScratch();

    void updateAutoScratch(float dt, Player& player);


private:
    sf::Texture cardTexture;
    sf::Sprite cardSprite;

    sf::Sprite baseSprite;

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

        sf::Sprite symbol;
    };

    std::vector<Zone> zones;

    void detectZones();
    sf::IntRect floodFill(unsigned int x, unsigned int y, std::vector<std::vector<bool>>& visited);
    void updateZoneClearedPixels(Zone& zone);
    void revealZone(Zone& zone, Player& player);

    int accumulatedMoney = 0;
    float accumulatedMultiplier = 1.f;
    bool winningsApplied = false;



    // symbols
    sf::Sprite emptySprite;
    sf::Sprite lucky7Sprite;

    // symbols count
	int emptyCount = 0;
	int lucky7Count = 0;



};
