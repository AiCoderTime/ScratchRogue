#include "ScratchCard.h"
#include "Prize.h"
#include "Player.h"
#include "Utils.h"
#include "ResourceManager.h"

#include <iostream>
#include <cmath>
#include <queue>
#include <random>
#include <sstream>
#include <iomanip>

// Constructor: load card textures, initialize sprites, and prepare zones/prizes
ScratchCard::ScratchCard(const std::string& cardPath, const std::string& overlayPath, float scale)
    : scale(scale), fullyRevealed(false), accumulatedMoney(0), accumulatedMultiplier(1.f), winningsApplied(false)
{
    static bool fontLoaded = false;

    // Load main font once for text rendering elsewhere
    if (!fontLoaded && !ResourceManager::loadFont("main", "assets/fonts/retro.ttf")) {
        std::cerr << "[Error] Failed to load 'main' font in ScratchCard.\n";
    }
    fontLoaded = true;

    // Load base card texture and setup sprite
    cardTexture.loadFromFile(cardPath);
    cardTexture.setSmooth(false);
    cardSprite.setTexture(cardTexture);
    cardSprite.setScale(scale, scale);

    // Load overlay image/texture for scratch zones and setup sprite
    overlayImage.loadFromFile(overlayPath);
    overlayTexture.loadFromImage(overlayImage);
    overlayTexture.setSmooth(false);
    overlaySprite.setTexture(overlayTexture);
    overlaySprite.setScale(scale, scale);

    // Initialize scratch mask to opaque overlay initially (no scratched pixels)
    scratchMask = overlayImage;

    // Load prize symbols from resource manager
    lucky7Sprite.setTexture(ResourceManager::getTexture("7"));
    emptySprite.setTexture(ResourceManager::getTexture("empty"));

    // Detect scratch zones and assign prizes randomly
    initializeZonesFromOverlay();
    assignRandomPrizes();
}

// Helper: Check if pixel at (x,y) in image is opaque (alpha > 0)
static bool isOpaque(const sf::Image& img, unsigned int x, unsigned int y) {
    return img.getPixel(x, y).a > 0;
}

// Set card and overlay sprites position on screen
void ScratchCard::setPosition(float x, float y) {
    cardSprite.setPosition(x, y);
    overlaySprite.setPosition(x, y);
    baseSprite.setPosition(x, y);  // Keep base sprite aligned too
}

// Return width of card in pixels accounting for scale
float ScratchCard::getWidth() const {
    return cardTexture.getSize().x * scale;
}

// Return height of card in pixels accounting for scale
float ScratchCard::getHeight() const {
    return cardTexture.getSize().y * scale;
}

// Attempt to scratch the card at world coordinates (x, y).
// Returns true if any pixels were scratched, false otherwise.
bool ScratchCard::scratchAt(float x, float y, Player& player) {
    // Convert world coordinates to local scratch mask coords
    int localX = static_cast<int>((x - overlaySprite.getPosition().x) / scale);
    int localY = static_cast<int>((y - overlaySprite.getPosition().y) / scale);

    // Scratch radius decreases with scale but never less than 1 pixel
    const int baseRadius = 8;
    int radius = std::max(1, static_cast<int>(baseRadius / scale));

    bool scratchedAny = false;

    // Loop over pixels in circle radius and clear alpha in scratch mask
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int px = localX + dx;
            int py = localY + dy;

            // Check bounds and circular radius
            if (px >= 0 && py >= 0 &&
                px < static_cast<int>(scratchMask.getSize().x) &&
                py < static_cast<int>(scratchMask.getSize().y) &&
                dx * dx + dy * dy <= radius * radius) {

                // Only scratch pixels that are not already transparent
                if (scratchMask.getPixel(px, py).a != 0) {
                    scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0)); // Clear pixel alpha
                    scratchedAny = true;
                }
            }
        }
    }

    if (!scratchedAny) return false; // Nothing scratched, early out

    // Update overlay texture with new scratch mask
    updateOverlayTexture();

    // Update all zones progress and reveal any fully scratched zones
    for (auto& zone : zones) {
        if (!zone.revealed) {
            updateZoneClearedPixels(zone);

            float percent = (zone.clearedPixels / static_cast<float>(zone.totalPixels)) * 100.f;
            if (percent >= 97.f) { // Threshold to reveal zone
                revealZone(zone, player);
            }
        }
    }

    // Check if entire card is fully revealed now
    if (!fullyRevealed && isFullyScratched()) {
        fullyRevealed = true;
        std::cout << "ScratchCard is fully revealed now!\n";
    }

    return true;
}

// Reveal a given zone fully (clear all pixels in zone) and apply its prize to player if not yet done
void ScratchCard::revealZone(Zone& zone, Player& player) {
    // Clear entire zone pixels in scratch mask
    for (int px = zone.rect.left; px < zone.rect.left + zone.rect.width; ++px) {
        for (int py = zone.rect.top; py < zone.rect.top + zone.rect.height; ++py) {
            scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0));
        }
    }

    zone.revealed = true;
    updateOverlayTexture();

    if (zone.applied) return; // Already applied prize for this zone

    // Apply prize effects based on prize type
    switch (zone.prize.type) {
    case PrizeType::Money:
        lucky7Count++; // Count money prize as lucky7 for matching logic
        std::cout << "[Debug] Lucky 7 revealed! Total so far: " << lucky7Count << "\n";
        break;
    case PrizeType::Multiplier:
        accumulatedMultiplier += zone.prize.multiplier;
        std::cout << "[Debug] Added Multiplier: " << zone.prize.multiplier
            << ", Total Multiplier: " << accumulatedMultiplier << "\n";
        break;
    case PrizeType::Relic:
        player.addRelic(zone.prize.relicId);
        std::cout << "[Debug] Added Relic: " << zone.prize.relicId << "\n";
        break;
    default:
        break;
    }

    zone.applied = true;
}

// Draw the base card sprite (big card image behind overlay)
void ScratchCard::drawBase(sf::RenderTarget& target) const {
    target.draw(baseSprite);
}

// Draw the overlay sprite (scratch mask on top)
void ScratchCard::drawOverlay(sf::RenderTarget& target) const {
    target.draw(overlaySprite);
}

// Draw prize symbols (e.g. lucky 7s) on their zones
void ScratchCard::drawPrizes(sf::RenderTarget& target) const {
    for (const auto& zone : zones) {
        float posX = overlaySprite.getPosition().x + zone.rect.left * scale;
        float posY = overlaySprite.getPosition().y + zone.rect.top * scale;
        float width = zone.rect.width * scale;
        float height = zone.rect.height * scale;

        sf::Sprite sprite;

        // Select sprite by prize type
        switch (zone.prize.type) {
        case PrizeType::Money: sprite = lucky7Sprite; break;
        case PrizeType::None:  sprite = emptySprite;  break;
        default: continue; // Skip Multiplier & Relic visuals here
        }

        // Scale sprite to fit within 80% of zone rect
        float scaleX = (width * 0.8f) / sprite.getTexture()->getSize().x;
        float scaleY = (height * 0.8f) / sprite.getTexture()->getSize().y;
        float finalScale = std::min(scaleX, scaleY);
        sprite.setScale(finalScale, finalScale);

        // Center sprite within zone
        float spriteW = sprite.getTexture()->getSize().x * finalScale;
        float spriteH = sprite.getTexture()->getSize().y * finalScale;
        sprite.setPosition(posX + (width - spriteW) / 2.f, posY + (height - spriteH) / 2.f);

        target.draw(sprite);
    }
}

// Return vector of PrizeTextInfo for multiplier prizes to draw their text labels
std::vector<PrizeTextInfo> ScratchCard::getRevealedPrizeTexts() const {
    std::vector<PrizeTextInfo> result;

    for (const auto& zone : zones) {
        if (zone.prize.type == PrizeType::Multiplier) {
            std::string txt = getPrizeText(zone.prize);
            float x = overlaySprite.getPosition().x + (zone.rect.left + zone.rect.width / 2.f) * scale;
            float y = overlaySprite.getPosition().y + (zone.rect.top + zone.rect.height / 2.f) * scale;
            result.push_back({ txt, sf::Vector2f(x, y) });
        }
    }

    return result;
}

// Apply accumulated winnings to player balance and reset counters
void ScratchCard::applyWinningsToPlayer(Player& player) {
    if (winningsApplied) return; // Avoid double applying

    // Determine base reward based on number of lucky 7 matches
    int baseReward = 0;
    switch (lucky7Count) {
    case 2: baseReward = 10; break;
    case 3: baseReward = 20; break;
    case 4: baseReward = 50; break;
    case 5: baseReward = 100; break;
    default: break;
    }

    // Calculate final reward with multiplier
    int finalReward = static_cast<int>(baseReward * accumulatedMultiplier);
    accumulatedMoney = finalReward; // Store reward

    if (finalReward > 0) {
        player.addBalance(finalReward);
        std::cout << "[Debug] Lucky 7s matched: " << lucky7Count
            << " ? Base: £" << baseReward
            << ", x" << accumulatedMultiplier
            << " = £" << finalReward << "\n";
    }
    else {
        std::cout << "[Debug] Not enough Lucky 7s to win a reward (count = " << lucky7Count << ").\n";
    }

    winningsApplied = true;

    // Reset multiplier and lucky7 counter for next card
    accumulatedMultiplier = 1.f;
    lucky7Count = 0;
}

// Randomly assign prizes to each detected zone on the card
void ScratchCard::assignRandomPrizes() {
    std::cout << "Assigning prizes to card at " << this << "\n";

    for (auto& zone : zones) {
        int r = Utils::randInt(0, 99);
        std::cout << "  Prize roll: " << r;

        if (r < 50) {
            std::cout << " ? Money\n";
            zone.prize = { PrizeType::Money };
        }
        else if (r < 90) {
            std::cout << " ? None\n";
            zone.prize = { PrizeType::None };
        }
        else {
            // Random multiplier between 1.5 and 2.5 approx.
            float mult = 1.5f + Utils::randFloat(0.f, 1.f);
            std::cout << " ? Multiplier: " << mult << "\n";
            zone.prize = { PrizeType::Multiplier, 0, mult };
        }
    }
}

// Instantly reveal all zones and clear scratch mask
void ScratchCard::revealAll() {
    for (unsigned int x = 0; x < scratchMask.getSize().x; ++x) {
        for (unsigned int y = 0; y < scratchMask.getSize().y; ++y) {
            scratchMask.setPixel(x, y, sf::Color(0, 0, 0, 0)); // Clear all pixels
        }
    }

    for (auto& zone : zones) {
        zone.revealed = true;
    }

    updateOverlayTexture();
    fullyRevealed = true;
}

// Returns true if card is fully revealed
bool ScratchCard::isFullyRevealed() const {
    return fullyRevealed;
}

// Calculate overall scratch completion percent (0-100)
float ScratchCard::getScratchCompletionPercent() const {
    int totalPixels = scratchMask.getSize().x * scratchMask.getSize().y;
    int cleared = 0;

    for (unsigned int x = 0; x < scratchMask.getSize().x; ++x) {
        for (unsigned int y = 0; y < scratchMask.getSize().y; ++y) {
            if (scratchMask.getPixel(x, y).a == 0) ++cleared;
        }
    }

    return (cleared / static_cast<float>(totalPixels)) * 100.f;
}

// Returns true if all zones are fully revealed
bool ScratchCard::isFullyScratched() const {
    for (const auto& zone : zones) {
        if (!zone.revealed) return false;
    }
    return true;
}

// Initialize zones vector by detecting opaque regions in overlay image
void ScratchCard::initializeZonesFromOverlay() {
    zones.clear();
    detectZones();
}

// Detect connected opaque regions as scratch zones using flood fill
void ScratchCard::detectZones() {
    unsigned int width = overlayImage.getSize().x;
    unsigned int height = overlayImage.getSize().y;

    // 2D visited array for flood fill
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));

    for (unsigned int x = 0; x < width; ++x) {
        for (unsigned int y = 0; y < height; ++y) {
            if (!visited[x][y] && isOpaque(overlayImage, x, y)) {
                sf::IntRect zoneRect = floodFill(x, y, visited);

                Zone zone;
                zone.rect = zoneRect;
                zone.revealed = false;
                zone.applied = false;

                // Count opaque pixels inside zone rect
                int count = 0;
                for (int px = zoneRect.left; px < zoneRect.left + zoneRect.width; ++px) {
                    for (int py = zoneRect.top; py < zoneRect.top + zoneRect.height; ++py) {
                        if (isOpaque(overlayImage, px, py)) count++;
                    }
                }

                zone.totalPixels = count;
                zone.clearedPixels = 0;

                zones.push_back(zone);
            }
        }
    }
}

// Flood fill algorithm to find connected opaque region starting at (startX, startY)
sf::IntRect ScratchCard::floodFill(unsigned int startX, unsigned int startY, std::vector<std::vector<bool>>& visited) {
    unsigned int width = overlayImage.getSize().x;
    unsigned int height = overlayImage.getSize().y;

    unsigned int minX = startX, maxX = startX, minY = startY, maxY = startY;
    std::queue<std::pair<unsigned int, unsigned int>> q;
    q.push({ startX, startY });
    visited[startX][startY] = true;

    // Directions for neighbors (left, right, up, down)
    const int dx[] = { 1, -1, 0, 0 };
    const int dy[] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            // Check bounds and visit opaque pixels not yet visited
            if (nx >= 0 && ny >= 0 && nx < static_cast<int>(width) && ny < static_cast<int>(height)) {
                if (!visited[nx][ny] && isOpaque(overlayImage, nx, ny)) {
                    visited[nx][ny] = true;
                    q.push({ (unsigned)nx, (unsigned)ny });

                    // Update bounding box
                    minX = std::min(minX, static_cast<unsigned>(nx));
                    maxX = std::max(maxX, static_cast<unsigned>(nx));
                    minY = std::min(minY, static_cast<unsigned>(ny));
                    maxY = std::max(maxY, static_cast<unsigned>(ny));
                }
            }
        }
    }

    return sf::IntRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

// Count how many pixels in zone are cleared (alpha == 0)
void ScratchCard::updateZoneClearedPixels(Zone& zone) {
    int cleared = 0;
    for (int px = zone.rect.left; px < zone.rect.left + zone.rect.width; ++px) {
        for (int py = zone.rect.top; py < zone.rect.top + zone.rect.height; ++py) {
            if (scratchMask.getPixel(px, py).a == 0) ++cleared;
        }
    }
    zone.clearedPixels = cleared;
}

// Update overlay texture from scratchMask image after scratching
void ScratchCard::updateOverlayTexture() {
    overlayTexture.update(scratchMask);
}

// Return string representation of prize to render as text
std::string ScratchCard::getPrizeText(const Prize& prize) const {
    if (prize.type == PrizeType::Money) {
        return "£" + std::to_string(prize.amount);
    }
    else if (prize.type == PrizeType::Multiplier) {
        std::ostringstream oss;
        oss << "x" << std::fixed << std::setprecision(1) << prize.multiplier;
        return oss.str();
    }
    return "";
}

// Load card texture by card ID, mapping shop preview IDs to large scratch card textures
void ScratchCard::loadCard(const std::string& cardId) {
    std::string actualCardId = cardId;

    // Example mapping for shop preview -> full card texture
    if (cardId == "lucky_7_shop") {
        actualCardId = "lucky_7";  // Load the full-size scratch card texture
    }
    // Add other mappings as needed

    sf::Texture& texture = ResourceManager::getTexture(actualCardId);
    baseSprite.setTexture(texture);
    baseSprite.setScale(scale, scale);

    // Align base sprite position with card sprite
    baseSprite.setPosition(cardSprite.getPosition());
}

// Reset scratch progress and all prizes
void ScratchCard::resetScratch() {
    scratchMask = overlayImage;    // Reset scratch mask to opaque
    updateOverlayTexture();

    for (auto& zone : zones) {
        zone.revealed = false;
        zone.applied = false;
        zone.clearedPixels = 0;
    }

    fullyRevealed = false;
    winningsApplied = false;
    accumulatedMoney = 0;
    accumulatedMultiplier = 1.f;
    lucky7Count = 0;
}

// Start auto scratch sequence (auto reveal zones with timer)
void ScratchCard::startAutoScratch() {
    if (autoScratchActive) return;
    autoScratchActive = true;
    autoScratchZoneIndex = 0;
    autoScratchTimer = 0.f;
    autoScratchInterval = 0.5f;  // Initial interval (seconds), speeds up later
}

// Update auto scratch process by delta time dt
void ScratchCard::updateAutoScratch(float dt, Player& player) {
    if (!autoScratchActive) return;

    autoScratchTimer += dt;
    if (autoScratchTimer >= autoScratchInterval) {
        autoScratchTimer = 0.f;

        if (autoScratchZoneIndex < zones.size()) {
            Zone& zone = zones[autoScratchZoneIndex];
            if (!zone.revealed) {
                revealZone(zone, player);
            }
            autoScratchZoneIndex++;

            // Speed up interval with acceleration factor but limit to minimum interval
            autoScratchInterval = std::max(autoScratchMinInterval, autoScratchInterval * autoScratchAcceleration);
        }
        else {
            // All zones auto scratched
            autoScratchActive = false;
            fullyRevealed = true;  // Mark card fully revealed
            std::cout << "Auto scratch complete for current card.\n";
        }
    }
}
