#pragma once
#include <vector>
#include <string>
#include <unordered_map>

// Represents the player state and inventory
class Player {
public:
    Player();

    // Get current currency balance
    int getBalance() const;

    // Add (or subtract) amount from balance; clamps at zero
    void addBalance(int amount);

    // Reset balance to zero
    void resetBalance();

    // Add a relic by name to the player's inventory
    void addRelic(const std::string& relicName);

    // Get all relics owned
    const std::vector<std::string>& getRelics() const;

    // Increase multiplier by given amount
    void addMultiplier(float multiplierAmount);

    // Get current multiplier value
    float getMultiplier() const;

    // Add a card by ID, incrementing count
    void addCard(const std::string& cardId);

    // Use one instance of card by ID; returns true if successful
    bool useCard(const std::string& cardId);

    // Get map of owned card IDs and their counts
    const std::unordered_map<std::string, int>& getOwnedCardsCount() const;

private:
    int balance = 0;  // Player's currency balance
    float multiplier = 1.0f;  // Multiplier applied to rewards, etc.

    std::vector<std::string> relics;  // Owned relics

    // Map card ID -> count owned
    std::unordered_map<std::string, int> ownedCardsCount;
};
