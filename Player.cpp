#include "Player.h"
#include <iostream>

Player::Player() : balance(0), multiplier(1.0f) {}

int Player::getBalance() const {
    return balance;
}

void Player::addBalance(int amount) {
    // Add amount (positive or negative) to balance, clamp at zero
    balance += amount;
    if (balance < 0) balance = 0;
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

void Player::addCard(const std::string& cardId) {
    ownedCardsCount[cardId]++;
}

bool Player::useCard(const std::string& cardId) {
    auto it = ownedCardsCount.find(cardId);
    if (it != ownedCardsCount.end() && it->second > 0) {
        --(it->second);
        if (it->second == 0) {
            ownedCardsCount.erase(it);
        }
        return true;
    }
    return false; // Player does not own the card
}

const std::unordered_map<std::string, int>& Player::getOwnedCardsCount() const {
    return ownedCardsCount;
}
