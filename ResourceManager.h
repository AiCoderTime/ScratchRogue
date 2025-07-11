#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <iostream>

// Static resource loader and cache manager for fonts and textures
class ResourceManager {
public:
    // Load a font from file and store it with the given name
    static bool loadFont(const std::string& name, const std::string& filename);

    // Retrieve a loaded font by name; returns default font if not found
    static sf::Font& getFont(const std::string& name);

    // Load a texture from file and store it with the given name
    static bool loadTexture(const std::string& name, const std::string& filename);

    // Retrieve a loaded texture by name; returns default texture if not found
    static sf::Texture& getTexture(const std::string& name);

private:
    // Resource storage
    static std::unordered_map<std::string, sf::Font> fonts;
    static std::unordered_map<std::string, sf::Texture> textures;

    // Defaults to return if requested resource is missing
    static sf::Font defaultFont;
    static sf::Texture defaultTexture;

    static bool defaultFontLoaded;
    static bool defaultTextureLoaded;
};
