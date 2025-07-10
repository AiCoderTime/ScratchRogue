#pragma once
#include <vector>
#include <string>

class Player {
public:
    Player();

    int getBalance() const;
    void addBalance(int amount);
    void resetBalance();

    void addRelic(const std::string& relicName);
    const std::vector<std::string>& getRelics() const;

    void addMultiplier(float multiplierAmount);
    float getMultiplier() const;

private:
    int balance;
    float multiplier;
    std::vector<std::string> relics;
};
