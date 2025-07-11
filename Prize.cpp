#include "Prize.h"
#include "Player.h"

// Applies the prize effect to the specified player
void Prize::applyToPlayer(Player& player) const {
    switch (type) {
    case PrizeType::Money:
        player.addBalance(amount);
        break;
    case PrizeType::Multiplier:
        player.addMultiplier(multiplier);
        break;
    case PrizeType::Relic:
        player.addRelic(relicId);
        break;
    case PrizeType::None:
    default:
        // No effect
        break;
    }
}
