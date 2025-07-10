#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <iostream>

class ResourceManager {
public:
    static bool loadFont(const std::string& name, const std::string& filename);
    static sf::Font& getFont(const std::string& name);

    static bool loadTexture(const std::string& name, const std::string& filename);
    static sf::Texture& getTexture(const std::string& name);

private:
    static std::unordered_map<std::string, sf::Font> fonts;
    static std::unordered_map<std::string, sf::Texture> textures;

    static sf::Font defaultFont;
    static sf::Texture defaultTexture;

    static bool defaultFontLoaded;
    static bool defaultTextureLoaded;
};
