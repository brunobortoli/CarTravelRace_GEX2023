#include "Assets.h"
#include <fstream>
#include <iostream>
#include "json.h"

void Assets::loadFromFile(const std::string& path)
{
    loadFonts(path);
    loadTextures(path);
    loadJson(path);
    loadAnimations(path);
}


void Assets::addFont(const std::string& fontName, const std::string& path)
{
    m_fontMap[fontName] = sf::Font();
    if (!m_fontMap[fontName].loadFromFile(path))
    {
        std::cerr << "Could not load Font from file: " << path << std::endl;
        m_fontMap.erase(fontName);
    }
    else
    {
        std::cout << "Loaded font: " << path << std::endl;
    }
}


const sf::Texture& Assets::getTexture(const std::string& textureName) const
{
    // assert(m_textureMap.contains(textureName)); not required .at() throws out_of_range excpt
    return m_textureMap.at(textureName);
}

const sf::Font& Assets::getFont(const std::string& fontName) const
{
    return m_fontMap.at(fontName);
}


void Assets::addTexture(const std::string& textureName, const std::string& path, bool smooth) {
    m_textureMap[textureName] = sf::Texture();
    if (!m_textureMap[textureName].loadFromFile(path))
    {
        std::cerr << "Could not load texture file: " << path << std::endl;
        m_textureMap.erase(textureName);
    }
    else
    {
        m_textureMap.at(textureName).setSmooth(smooth);
        std::cout << "Loaded texture: " << path << std::endl;
    }
}


void Assets::loadFonts(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "Font")
        {
            std::string name, path;
            confFile >> name >> path;
            addFont(name, path);
        }
        else
        {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}


void Assets::loadTextures(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "Texture")
        {
            std::string name, path;
            confFile >> name >> path;
            addTexture(name, path);
        }
        else
        {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();

}


void Assets::loadJson(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "JSON")
        {
            using json = nlohmann::json;

            std::string  path;
            confFile >> path;

            // read the FrameSets from the json file
            std::ifstream f(path);
            json data = json::parse(f)["frames"];

            for (auto i : data) {

                // clean up animation name
                std::string tmp = i["filename"];
                std::string::size_type n = tmp.find(" (");
                if (n == std::string::npos)
                    n = tmp.find(".png");

                // create IntRect for each frame in animation
                auto ir = sf::IntRect(i["frame"]["x"], i["frame"]["y"],
                    i["frame"]["w"], i["frame"]["h"]);

                m_frameSets[tmp.substr(0, n)].push_back(ir);
            }
            f.close();
        }
        else
        {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();

}


void Assets::loadAnimations(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "Animation")
        {
            std::string name, texture, repeat;
            float speed;
            confFile >> name >> texture >> speed >> repeat;

            Animation a(name,
                getTexture(texture),
                m_frameSets[name],
                sf::seconds(1 / speed),
                (repeat == "yes"));

            m_animationMap[name] = a;
        }
        else
        {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}


const Animation& Assets::getAnimation(const std::string& name) const {
    return m_animationMap.at(name);
}



