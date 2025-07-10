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
