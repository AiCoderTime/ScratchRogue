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
    : scale(scale)
{
    static bool fontLoaded = false;
    if (!fontLoaded) {
        if (!ResourceManager::loadFont("main", "assets/fonts/retro.ttf")) {
            std::cerr << "[Error] Failed to load 'main' font in ScratchCard.\n";
        }
        fontLoaded = true;
    }

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

    initializeZonesFromOverlay();
    assignRandomPrizes();

    winningsApplied = false;

}

void ScratchCard::setPosition(float x, float y) {
    cardSprite.setPosition(x, y);
    overlaySprite.setPosition(x, y);
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
    int radius = static_cast<int>(baseRadius / scale);
    if (radius < 1) radius = 1;

    bool scratchedAny = false;

    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int px = localX + dx;
            int py = localY + dy;

            if (px >= 0 && py >= 0 &&
                px < static_cast<int>(scratchMask.getSize().x) &&
                py < static_cast<int>(scratchMask.getSize().y) &&
                dx * dx + dy * dy <= radius * radius) {

                sf::Color current = scratchMask.getPixel(px, py);
                if (current.a != 0) {
                    scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0));
                    scratchedAny = true;
                }
            }
        }
    }

    if (scratchedAny) {
        updateOverlayTexture();

        for (auto& zone : zones) {
            if (!zone.revealed) {
                updateZoneClearedPixels(zone);
                float percent = (zone.clearedPixels / (float)zone.totalPixels) * 100.f;
                if (percent >= 97.f) {
                    revealZone(zone, player);

                    std::cout << "Zone revealed! Prize: ";
                    switch (zone.prize.type) {
                    case PrizeType::Money:
                        std::cout << "Money $" << zone.prize.amount << std::endl;
                        break;
                    case PrizeType::Multiplier:
                        std::cout << "Multiplier x" << zone.prize.multiplier << std::endl;
                        break;
                    case PrizeType::Relic:
                        std::cout << "Relic ID: " << zone.prize.relicId << std::endl;
                        break;
                    case PrizeType::None:
                    default:
                        std::cout << "No Prize" << std::endl;
                        break;
                    }
                }
            }
        }

        // Check if fully revealed now
        if (!fullyRevealed && isFullyScratched()) {
            fullyRevealed = true;
            std::cout << "ScratchCard is fully revealed now!" << std::endl;
        }
    }

    return scratchedAny;
}

void ScratchCard::updateOverlayTexture() {
    overlayTexture.update(scratchMask);
}

void ScratchCard::draw(sf::RenderTarget& target) const {
    target.draw(cardSprite);
    target.draw(overlaySprite);
}

std::vector<PrizeTextInfo> ScratchCard::getRevealedPrizeTexts() const {
    std::vector<PrizeTextInfo> result;
    for (const auto& zone : zones) {
        if (zone.revealed) {
            std::string txt = getPrizeText(zone.prize);
            if (!txt.empty()) {
                float x = (zone.rect.left + zone.rect.width / 2.f) * scale + overlaySprite.getPosition().x;
                float y = (zone.rect.top + zone.rect.height / 2.f) * scale + overlaySprite.getPosition().y;
                result.push_back({ txt, sf::Vector2f(x, y) });
            }
        }
    }
    return result;
}


float ScratchCard::getScratchCompletionPercent() const {
    int totalPixels = scratchMask.getSize().x * scratchMask.getSize().y;
    int clearedPixels = 0;

    for (unsigned int x = 0; x < scratchMask.getSize().x; ++x) {
        for (unsigned int y = 0; y < scratchMask.getSize().y; ++y) {
            if (scratchMask.getPixel(x, y).a == 0) {
                clearedPixels++;
            }
        }
    }
    return (clearedPixels / (float)totalPixels) * 100.f;
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

bool ScratchCard::isFullyRevealed() const {
    return fullyRevealed;
}

static bool isOpaque(const sf::Image& img, unsigned int x, unsigned int y) {
    return img.getPixel(x, y).a > 0;
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

    unsigned int minX = startX, maxX = startX;
    unsigned int minY = startY, maxY = startY;

    std::queue<std::pair<unsigned int, unsigned int>> q;
    q.push({ startX, startY });
    visited[startX][startY] = true;

    const int dx[] = { 1, -1, 0, 0 };
    const int dy[] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (int dir = 0; dir < 4; ++dir) {
            int nx = int(x) + dx[dir];
            int ny = int(y) + dy[dir];

            if (nx >= 0 && ny >= 0 && nx < int(width) && ny < int(height)) {
                if (!visited[nx][ny] && isOpaque(overlayImage, nx, ny)) {
                    visited[nx][ny] = true;
                    q.push({ (unsigned int)nx, (unsigned int)ny });

                    if (nx < int(minX)) minX = nx;
                    if (nx > int(maxX)) maxX = nx;
                    if (ny < int(minY)) minY = ny;
                    if (ny > int(maxY)) maxY = ny;
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
            if (scratchMask.getPixel(px, py).a == 0) {
                cleared++;
            }
        }
    }
    zone.clearedPixels = cleared;
}

void ScratchCard::revealZone(Zone& zone, Player& player) {
    for (int px = zone.rect.left; px < zone.rect.left + zone.rect.width; ++px) {
        for (int py = zone.rect.top; py < zone.rect.top + zone.rect.height; ++py) {
            scratchMask.setPixel(px, py, sf::Color(0, 0, 0, 0));
        }
    }
    zone.revealed = true;
    updateOverlayTexture();

    if (!zone.applied) {
        switch (zone.prize.type) {
        case PrizeType::Money:
            accumulatedMoney += zone.prize.amount;
            std::cout << "[Debug] Added Money: " << zone.prize.amount
                << ", Total Accumulated: " << accumulatedMoney << std::endl;
            break;
        case PrizeType::Multiplier:
            accumulatedMultiplier += zone.prize.multiplier;
            std::cout << "[Debug] Added Multiplier: " << zone.prize.multiplier
                << ", Total Multiplier: " << accumulatedMultiplier << std::endl;
            break;
        case PrizeType::Relic:
            player.addRelic(zone.prize.relicId);
            std::cout << "[Debug] Added Relic: " << zone.prize.relicId << std::endl;
            break;
        case PrizeType::None:
            break;
        }
        zone.applied = true;
    }
}

void ScratchCard::assignRandomPrizes() {
    for (auto& zone : zones) {
        Prize prize;
        int r = Utils::randInt(0, 99);

        if (r < 60) {
            prize.type = PrizeType::Money;
            prize.amount = 1 + Utils::randInt(0, 49);
        }
        else if (r < 85) {
            prize.type = PrizeType::Multiplier;
            prize.multiplier = 1.5f + Utils::randFloat(0.f, 1.f); // 1.5 to 2.5 approx
        }
        else if (r < 95) {
            prize.type = PrizeType::Relic;
            prize.relicId = "Relic_" + std::to_string(Utils::randInt(0, 9));
        }
        else {
            prize.type = PrizeType::None;
        }

        zone.prize = prize;
    }
}

void ScratchCard::applyWinningsToPlayer(Player& player) {
    if (winningsApplied) {
        std::cout << "[Debug] Winnings already applied, skipping.\n";
        return;
    }

    int finalMoney = static_cast<int>(accumulatedMoney * accumulatedMultiplier);

    std::cout << "[Debug] Applying winnings to player: "
        << "Money = " << accumulatedMoney
        << ", Multiplier = " << accumulatedMultiplier
        << ", Final money = " << finalMoney
        << std::endl;

    player.addBalance(finalMoney);

    winningsApplied = true;

    // Reset accumulators so they don't apply multiple times
    accumulatedMoney = 0;
    accumulatedMultiplier = 1.f;
}

std::string ScratchCard::getPrizeText(const Prize& prize) const {
    if (prize.type == PrizeType::Money) {
        return "\u00a3" + std::to_string(prize.amount); // £ symbol
    }
    else if (prize.type == PrizeType::Multiplier) {
        std::ostringstream oss;
        oss << "x" << std::fixed << std::setprecision(1) << prize.multiplier;
        return oss.str();
    }
    return "";
}

bool ScratchCard::isFullyScratched() const {
    for (const auto& zone : zones) {
        if (!zone.revealed) return false;
    }
    return true;
}
