#include "ResourceManager.h"

std::unordered_map<std::string, sf::Font> ResourceManager::fonts;
std::unordered_map<std::string, sf::Texture> ResourceManager::textures;

sf::Font ResourceManager::defaultFont;
sf::Texture ResourceManager::defaultTexture;

bool ResourceManager::defaultFontLoaded = false;
bool ResourceManager::defaultTextureLoaded = false;

bool ResourceManager::loadFont(const std::string& name, const std::string& filename) {
    sf::Font font;
    if (!font.loadFromFile(filename)) {
        std::cerr << "[Error] Failed to load font: " << filename << std::endl;
        return false;
    }
    fonts[name] = std::move(font);

    // Load default font if not loaded
    if (!defaultFontLoaded) {
        defaultFont = fonts[name];
        defaultFontLoaded = true;
    }

    return true;
}

sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        return it->second;
    }
    else {
        std::cerr << "[Warning] Font not found: " << name << ". Returning default font." << std::endl;
        if (!defaultFontLoaded) {
            // Load a system fallback font here or just leave defaultFont empty
            std::cerr << "[Error] Default font not loaded! Returning empty font." << std::endl;
        }
        return defaultFont;
    }
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& filename) {
    std::cout << "[Debug] Attempting to load texture: '" << name
        << "' from file: " << filename << std::endl;

    sf::Texture texture;
    if (!texture.loadFromFile(filename)) {
        std::cerr << "[Error] Failed to load texture: " << filename << std::endl;
        return false;
    }

    textures[name] = std::move(texture);
    std::cout << "[Success] Loaded texture '" << name << "' from: " << filename << std::endl;

    // Load default texture if not loaded
    if (!defaultTextureLoaded) {
        defaultTexture = textures[name];
        defaultTextureLoaded = true;
        std::cout << "[Info] Set default texture to: " << name << std::endl;
    }

    return true;
}


sf::Texture& ResourceManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second;
    }
    else {
        std::cerr << "[Warning] Texture not found: " << name << ". Returning default texture." << std::endl;
        if (!defaultTextureLoaded) {
            std::cerr << "[Error] Default texture not loaded! Returning empty texture." << std::endl;
        }
        return defaultTexture;
    }
}
