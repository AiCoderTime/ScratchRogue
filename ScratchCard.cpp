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

ScratchCard::ScratchCard(const std::string& cardPath, const std::string& overlayPath, float scale)
    : scale(scale), fullyRevealed(false), accumulatedMoney(0), accumulatedMultiplier(1.f), winningsApplied(false)
{
    static bool fontLoaded = false;
    if (!fontLoaded && !ResourceManager::loadFont("main", "assets/fonts/retro.ttf")) {
        std::cerr << "[Error] Failed to load 'main' font in ScratchCard.\n";
    }
    fontLoaded = true;

    cardTexture.loadFromFile(cardPath);
    cardTexture.setSmooth(false);
    cardSprite.setTexture(cardTexture);
    cardSprite.setScale(scale, scale);

    overlayImage.loadFromFile(overlayPath);
    overlayTexture.loadFromImage(overlayImage);
    overlayTexture.setSmooth(false);
    overlaySprite.setTexture(overlayTexture);
    overlaySprite.setScale(scale, scale);

    scratchMask = overlayImage;

    lucky7Sprite.setTexture(ResourceManager::getTexture("7"));
    emptySprite.setTexture(ResourceManager::getTexture("empty"));

    initializeZonesFromOverlay();
    assignRandomPrizes();
}


static bool isOpaque(const sf::Image& img, unsigned int x, unsigned int y) {
    return img.getPixel(x, y).a > 0;
}


void ScratchCard::setPosition(float x, float y) {
    cardSprite.setPosition(x, y);
    overlaySprite.setPosition(x, y);
    baseSprite.setPosition(x, y);  // <--- add this line
}


float ScratchCard::getWidth() const {
    return cardTexture.getSize().x * scale;
}

float ScratchCard::getHeight() const {
    return cardTexture.getSize().y * scale;
}

bool ScratchCard::scratchAt(float x, float y, Player& player) {
    int localX = static_cast<int>((x - overlaySprite.getPosition().x) / scale);
    int localY = static_cast<int>((y - overlaySprite.getPosition().y) / scale);

    const int baseRadius = 8;
    int radius = std::max(1, static_cast<int>(baseRadius / scale));

    bool scratchedAny = false;

    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int px = localX + dx;
            int py = localY + dy;

            if (px >= 0 && py >= 0 &&
                px < static_cast<int>(scratchMask.getSize().x) &&
                py < static_cast<int>(scratchMask.getSize().y) &&
                dx * dx + dy * dy <= radius * radius) {

                if (scratchMask.getPixel(px, py).a != 0) {
                    scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0));
                    scratchedAny = true;
                }
            }
        }
    }

    if (!scratchedAny) return false;

    updateOverlayTexture();

    for (auto& zone : zones) {
        if (!zone.revealed) {
            updateZoneClearedPixels(zone);
            float percent = (zone.clearedPixels / static_cast<float>(zone.totalPixels)) * 100.f;
            if (percent >= 97.f) {
                revealZone(zone, player);
            }
        }
    }

    if (!fullyRevealed && isFullyScratched()) {
        fullyRevealed = true;
        std::cout << "ScratchCard is fully revealed now!\n";
    }

    return true;
}

void ScratchCard::revealZone(Zone& zone, Player& player) {
    for (int px = zone.rect.left; px < zone.rect.left + zone.rect.width; ++px) {
        for (int py = zone.rect.top; py < zone.rect.top + zone.rect.height; ++py) {
            scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0));
        }
    }

    zone.revealed = true;
    updateOverlayTexture();

    if (zone.applied) return;

    switch (zone.prize.type) {
    case PrizeType::Money:
        lucky7Count++;
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

void ScratchCard::drawBase(sf::RenderTarget& target) const {
    target.draw(baseSprite);
}

void ScratchCard::drawOverlay(sf::RenderTarget& target) const {
    target.draw(overlaySprite);
}

void ScratchCard::drawPrizes(sf::RenderTarget& target) const {
    for (const auto& zone : zones) {
        float posX = overlaySprite.getPosition().x + zone.rect.left * scale;
        float posY = overlaySprite.getPosition().y + zone.rect.top * scale;
        float width = zone.rect.width * scale;
        float height = zone.rect.height * scale;

        sf::Sprite sprite;

        switch (zone.prize.type) {
        case PrizeType::Money: sprite = lucky7Sprite; break;
        case PrizeType::None:  sprite = emptySprite;  break;
        default: continue; // Skip Multiplier & Relic visual here
        }

        float scaleX = (width * 0.8f) / sprite.getTexture()->getSize().x;
        float scaleY = (height * 0.8f) / sprite.getTexture()->getSize().y;
        float finalScale = std::min(scaleX, scaleY);
        sprite.setScale(finalScale, finalScale);

        float spriteW = sprite.getTexture()->getSize().x * finalScale;
        float spriteH = sprite.getTexture()->getSize().y * finalScale;

        sprite.setPosition(posX + (width - spriteW) / 2.f, posY + (height - spriteH) / 2.f);
        target.draw(sprite);
    }
}

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

void ScratchCard::applyWinningsToPlayer(Player& player) {
    if (winningsApplied) return;

    int baseReward = 0;
    switch (lucky7Count) {
    case 2: baseReward = 10; break;
    case 3: baseReward = 20; break;
    case 4: baseReward = 50; break;
    case 5: baseReward = 100; break;
    default: break;
    }

    int finalReward = static_cast<int>(baseReward * accumulatedMultiplier);
    accumulatedMoney = finalReward; 

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
    accumulatedMultiplier = 1.f;
    lucky7Count = 0;
}


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
            float mult = 1.5f + Utils::randFloat(0.f, 1.f);
            std::cout << " ? Multiplier: " << mult << "\n";
            zone.prize = { PrizeType::Multiplier, 0, mult };
        }
    }
}

void ScratchCard::revealAll() {
    for (unsigned int x = 0; x < scratchMask.getSize().x; ++x) {
        for (unsigned int y = 0; y < scratchMask.getSize().y; ++y) {
            scratchMask.setPixel(x, y, sf::Color(0, 0, 0, 0));
        }
    }

    for (auto& zone : zones) {
        zone.revealed = true;
    }

    updateOverlayTexture();
    fullyRevealed = true;
}

bool ScratchCard::isFullyRevealed() const
{
    return fullyRevealed;
}

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

bool ScratchCard::isFullyScratched() const {
    for (const auto& zone : zones) {
        if (!zone.revealed) return false;
    }
    return true;
}

void ScratchCard::initializeZonesFromOverlay() {
    zones.clear();
    detectZones();
}

void ScratchCard::detectZones() {
    unsigned int width = overlayImage.getSize().x;
    unsigned int height = overlayImage.getSize().y;
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));

    for (unsigned int x = 0; x < width; ++x) {
        for (unsigned int y = 0; y < height; ++y) {
            if (!visited[x][y] && isOpaque(overlayImage, x, y)) {
                sf::IntRect zoneRect = floodFill(x, y, visited);
                Zone zone;
                zone.rect = zoneRect;
                zone.revealed = false;
                zone.applied = false;

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

sf::IntRect ScratchCard::floodFill(unsigned int startX, unsigned int startY, std::vector<std::vector<bool>>& visited) {
    unsigned int width = overlayImage.getSize().x;
    unsigned int height = overlayImage.getSize().y;

    unsigned int minX = startX, maxX = startX, minY = startY, maxY = startY;
    std::queue<std::pair<unsigned int, unsigned int>> q;
    q.push({ startX, startY });
    visited[startX][startY] = true;

    const int dx[] = { 1, -1, 0, 0 };
    const int dy[] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (nx >= 0 && ny >= 0 && nx < static_cast<int>(width) && ny < static_cast<int>(height)) {
                if (!visited[nx][ny] && isOpaque(overlayImage, nx, ny)) {
                    visited[nx][ny] = true;
                    q.push({ (unsigned)nx, (unsigned)ny });

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

void ScratchCard::updateZoneClearedPixels(Zone& zone) {
    int cleared = 0;
    for (int px = zone.rect.left; px < zone.rect.left + zone.rect.width; ++px) {
        for (int py = zone.rect.top; py < zone.rect.top + zone.rect.height; ++py) {
            if (scratchMask.getPixel(px, py).a == 0) ++cleared;
        }
    }
    zone.clearedPixels = cleared;
}

void ScratchCard::updateOverlayTexture() {
    overlayTexture.update(scratchMask);
}

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

// --- FIXED: loadCard to map shop preview IDs to big scratch card textures ---
void ScratchCard::loadCard(const std::string& cardId) {
    std::string actualCardId = cardId;

    // Map shop preview card IDs to their large full card texture equivalents
    if (cardId == "lucky_7_shop") {
        actualCardId = "lucky_7";  // Load the big scratch card sprite here
    }
    // Add other card ID mappings here if needed

    sf::Texture& texture = ResourceManager::getTexture(actualCardId);
    baseSprite.setTexture(texture);
    baseSprite.setScale(scale, scale);

    // Keep baseSprite positioned same as cardSprite (you might want to adjust this if needed)
    baseSprite.setPosition(cardSprite.getPosition());
}
// ---

void ScratchCard::resetScratch() {
    // Reset scratch progress variables and mask texture
    scratchMask = overlayImage;
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

void ScratchCard::startAutoScratch() {
    if (autoScratchActive) return;
    autoScratchActive = true;
    autoScratchZoneIndex = 0;
    autoScratchTimer = 0.f;
    autoScratchInterval = 0.5f;  // start slower, speeds up
}

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

            // Ramp up speed by reducing interval
            autoScratchInterval = std::max(autoScratchMinInterval, autoScratchInterval * autoScratchAcceleration);
        }
        else {
            // Finished auto scratch
            autoScratchActive = false;
            fullyRevealed = true;  // mark card fully revealed for Game to detect
            std::cout << "Auto scratch complete for current card.\n";
        }
    }
}

