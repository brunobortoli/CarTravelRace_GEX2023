#ifndef SFMLCLASS_GAMEENGINE_H
#define SFMLCLASS_GAMEENGINE_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "EntityManager.h"
#include "Assets.h"
#include <map>
#include <string>
#include <functional>



class Scene;
enum class SceneID { NONE, MENU, GEO, FTR, MINS };

using Sptr = std::shared_ptr<Scene>;
using SceneMap = std::map<SceneID, Sptr>;
using FactoryMap = std::map<SceneID, std::function<Sptr()>>;

class GameEngine {
private:
    sf::Vector2u            m_windowSize{ 1280, 768 };
    sf::RenderWindow        m_window;
    Assets                  m_assets;

    SceneID                 m_currentScene;
    SceneMap                m_sceneMap;
    FactoryMap              m_factories;

    bool                    m_isRunning{ true };


    // stats
    sf::Text                m_statisticsText;
    sf::Time                m_statisticsUpdateTime{ sf::Time::Zero };
    unsigned int            m_statisticsNumFrames{ 0 };
    void                    updateStatistics(sf::Time dt);

    void                    init(const std::string& path);
    void                    sUserInput();
    std::shared_ptr<Scene>  currentScene();
    void                    createFactories();
    void                    createMenu();
    void                    createMenuInstuc();

    const static sf::Time   TIME_PER_FRAME;

public:
    GameEngine(const std::string& configPath);

    void                    run();
    void                    quit();
    sf::RenderWindow& window();
    const Assets& assets() const;
    bool                    isRunning();
    void                    changeScene(SceneID, bool endCurrentScene = false);

    void                    quitLevel();
    void                    backLevel();
};


#endif //SFMLCLASS_GAMEENGINE_H
