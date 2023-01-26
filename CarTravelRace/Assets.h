#ifndef SFMLCLASS_ASSETS_H
#define SFMLCLASS_ASSETS_H


#include <string>
#include <map>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include "Animation.h"

class Assets {
private:
    std::map<std::string, sf::Texture>	            m_textureMap;
    std::map<std::string, sf::Font>		            m_fontMap;
    std::map<std::string, Animation>                m_animationMap;
    std::map<std::string, std::vector<sf::IntRect>> m_frameSets;


    void addTexture(const std::string& textureName, const std::string& path, bool smooth = true);
    void addFont(const std::string& fontName, const std::string& path);

    void loadFonts(const std::string& path);
    void loadTextures(const std::string& path);
    void loadJson(const std::string& path);
    void loadAnimations(const std::string& path);



public:

    Assets() = default;

    void loadFromFile(const std::string& path);

    const sf::Texture& getTexture(const std::string& textureName) const;
    const sf::Font& getFont(const std::string& fontName) const;
    const Animation& getAnimation(const std::string& name) const;

};


#endif //SFMLCLASS_ASSETS_H
