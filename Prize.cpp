#include "Prize.h"
#include "Player.h"

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
        break;
    }
}
