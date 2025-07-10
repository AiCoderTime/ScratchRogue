#include "Player.h"
#include <iostream>

Player::Player() : balance(0), multiplier(1.0f) {}

int Player::getBalance() const {
    return balance;
}

void Player::addBalance(int amount) {
    std::cout << "[Player] Adding " << amount << " to balance (current: " << balance << ")" << std::endl;
    balance += amount;
    if (balance < 0)
        balance = 0;
    std::cout << "[Player] New balance: " << balance << std::endl;
}

void Player::resetBalance() {
    balance = 0;
}

void Player::addRelic(const std::string& relicName) {
    relics.push_back(relicName);
}

const std::vector<std::string>& Player::getRelics() const {
    return relics;
}

void Player::addMultiplier(float multiplierAmount) {
    multiplier += multiplierAmount;
}

float Player::getMultiplier() const {
    return multiplier;
}

// Add card count, instead of just adding to vector
void Player::addCard(const std::string& cardId) {
    ownedCardsCount[cardId]++;
    std::cout << "Added card " << cardId << ". Now owned: " << ownedCardsCount[cardId] << std::endl;
}

// Remove card (decrement count)
bool Player::useCard(const std::string& cardId) {
    auto it = ownedCardsCount.find(cardId);
    if (it != ownedCardsCount.end() && it->second > 0) {
        it->second--;
        if (it->second == 0) {
            ownedCardsCount.erase(it);
        }
        std::cout << "Used card " << cardId << ". Remaining: " << (it == ownedCardsCount.end() ? 0 : it->second) << std::endl;
        return true;
    }
    return false; // no card to use
}

// Getter for owned cards with counts
const std::unordered_map<std::string, int>& Player::getOwnedCardsCount() const {
    return ownedCardsCount;
}
