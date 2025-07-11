#pragma once
#include <string>

// Types of prizes that can be awarded
enum class PrizeType {
    None,        // No prize
    Money,       // Monetary reward
    Multiplier,  // Multiplier bonus
    Relic        // Grant a relic item
};

// Represents a prize to be applied to a player
struct Prize {
    PrizeType type = PrizeType::None;  // Prize category
    int amount = 0;                    // Amount of money if Money type
    float multiplier = 1.f;            // Multiplier value if Multiplier type
    std::string relicId = "";          // Relic ID if Relic type

    // Apply this prize effect to the given player
    void applyToPlayer(class Player& player) const;
};
