#pragma once
#include <vector>
#include <string>
#include <unordered_map>  // Add this for unordered_map

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

    void addCard(const std::string& cardId);
    bool useCard(const std::string& cardId);

    const std::unordered_map<std::string, int>& getOwnedCardsCount() const;

private:
    int balance;
    float multiplier;
    std::vector<std::string> relics;

    std::unordered_map<std::string, int> ownedCardsCount;
};
