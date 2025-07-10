#pragma once
#include <string>

enum class PrizeType {
    None,
    Money,
    Multiplier,
    Relic
};

struct Prize {
    PrizeType type = PrizeType::None;
    int amount = 0;
    float multiplier = 1.f;
    std::string relicId = "";

    void applyToPlayer(class Player& player) const;
};
