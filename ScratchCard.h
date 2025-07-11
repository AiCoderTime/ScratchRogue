#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include "Prize.h"

// Forward declaration to avoid circular dependency
class Player;

// Struct for prize text and position info used for rendering text on card
struct PrizeTextInfo {
    std::string text;
    sf::Vector2f position;
};

class ScratchCard {
public:
    // Constructor: loads card and overlay textures, sets scale
    ScratchCard(const std::string& cardPath, const std::string& overlayPath, float scale);

    // Attempt to scratch at given coordinates, returns true if scratch occurred
    bool scratchAt(float x, float y, Player& player);

    // Drawing functions to separate base card and scratch overlay
    void drawBase(sf::RenderTarget& target) const;
    void drawOverlay(sf::RenderTarget& target) const;

    // Set card position on screen
    void setPosition(float x, float y);

    // Get card dimensions considering scale
    float getWidth() const;
    float getHeight() const;

    // Get prize text info for revealed multiplier zones for rendering
    std::vector<PrizeTextInfo> getRevealedPrizeTexts() const;

    // Percentage of the card scratched off (0 to 100)
    float getScratchCompletionPercent() const;

    // Reveal all zones instantly
    void revealAll();

    // Check if card is fully revealed
    bool isFullyRevealed() const;

    // Prepare zones from overlay image (called on load/reset)
    void initializeZonesFromOverlay();

    // Randomly assign prizes to each zone
    void assignRandomPrizes();

    // Check if all zones are fully revealed
    bool isFullyScratched() const;

    // Get total accumulated money prize
    int getAccumulatedMoney() const { return accumulatedMoney; }

    // Get accumulated multiplier prize
    float getAccumulatedMultiplier() const { return accumulatedMultiplier; }

    // Apply winnings (money/multiplier/relics) to player balance and stats
    void applyWinningsToPlayer(Player& player);

    // Draw prize symbols on the card
    void drawPrizes(sf::RenderTarget& target) const;

    // Get textual representation of a prize (e.g. "£10", "x1.5")
    std::string getPrizeText(const Prize& prize) const;

    // Check if winnings have already been applied (to avoid duplicates)
    bool areWinningsApplied() const { return winningsApplied; }

    // Load card textures by card ID (supports mapping shop preview IDs to big textures)
    void loadCard(const std::string& cardId);

    // Reset scratch progress and prizes
    void resetScratch();

    // === Auto Scratch control variables and methods ===

    bool autoScratchActive = false;          // Is auto-scratch currently running
    size_t autoScratchZoneIndex = 0;         // Current zone being auto-scratched
    float autoScratchTimer = 0.f;             // Timer for auto-scratch interval
    float autoScratchInterval = 0.5f;         // Initial delay between zones (seconds)
    float autoScratchAcceleration = 0.9f;     // Multiplier to ramp speed up (reduces interval)
    float autoScratchMinInterval = 0.05f;     // Minimum interval between zones

    // Start auto scratch process
    void startAutoScratch();

    // Update auto scratch, reveal zones automatically
    void updateAutoScratch(float dt, Player& player);

private:
    sf::Texture cardTexture;       // Card base texture (small or large)
    sf::Sprite cardSprite;         // Card sprite for positioning and drawing

    sf::Sprite baseSprite;         // Base sprite that displays large card texture for scratching

    sf::Image overlayImage;        // Overlay image used to define scratchable zones (alpha channel)
    sf::Texture overlayTexture;    // Overlay texture updated with scratch mask
    sf::Sprite overlaySprite;      // Overlay sprite drawn on top of baseSprite

    sf::Image scratchMask;         // Image mask tracking scratched pixels (alpha 0 = scratched)

    float scale;                   // Scale applied to card and overlay sprites
    bool fullyRevealed = false;    // Flag indicating card is fully revealed

    // Update overlay texture from current scratch mask image
    void updateOverlayTexture();

    // Represents a scratch zone on the card
    struct Zone {
        sf::IntRect rect;          // Rectangular bounds of the zone
        int totalPixels = 0;       // Total opaque pixels in zone
        int clearedPixels = 0;     // Currently scratched pixels in zone
        bool revealed = false;     // Has zone been revealed (fully scratched)
        bool applied = false;      // Have prizes in this zone been applied to player
        Prize prize;               // Prize assigned to this zone

        sf::Sprite symbol;         // Optional sprite for prize symbol
    };

    std::vector<Zone> zones;       // All scratch zones on the card

    // Detect scratch zones from overlay image alpha regions
    void detectZones();

    // Flood fill to find connected opaque pixels for zone detection
    sf::IntRect floodFill(unsigned int x, unsigned int y, std::vector<std::vector<bool>>& visited);

    // Update cleared pixel count for given zone
    void updateZoneClearedPixels(Zone& zone);

    // Reveal a zone fully and apply prize if not already done
    void revealZone(Zone& zone, Player& player);

    // Accumulated winnings state
    int accumulatedMoney = 0;
    float accumulatedMultiplier = 1.f;
    bool winningsApplied = false;

    // Cached symbols for prizes
    sf::Sprite emptySprite;
    sf::Sprite lucky7Sprite;

    // Counts of prize symbols found
    int emptyCount = 0;
    int lucky7Count = 0;
};
