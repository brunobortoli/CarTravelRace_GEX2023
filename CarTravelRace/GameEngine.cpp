#include "GameEngine.h"
#include "Components.h"
#include "Entity.h"
#include "Scene_GexFighter.h"

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "Scene_Game.h"
#include "Scene_Menu.h"
#include "MusicPlayer.h"
#include "SoundPlayer.h"


const sf::Time GameEngine::TIME_PER_FRAME = sf::seconds((1.f / 60.f));


GameEngine::GameEngine(const std::string& configPath) {
    init(configPath);
    m_assets.loadFromFile(configPath);
    m_window.create(sf::VideoMode(m_windowSize.x, m_windowSize.y), "CAR TRAVEL RACE");

    m_statisticsText.setFont(m_assets.getFont("Star"));
    m_statisticsText.setPosition(15.0f, 15.0f);
    m_statisticsText.setCharacterSize(15);

    createFactories();
    createMenu();
    createMenuInstuc();
    m_currentScene = SceneID::MENU;
    MusicPlayer::getInstance().play("menuTheme");
}


void GameEngine::createMenu() {

    // create the menu_scene and put in sceneMap
    auto menuScene = std::make_shared<Scene_Menu>(this);
    m_sceneMap[SceneID::MENU] = menuScene;

    // add items to menu_scene
    menuScene->registerItem(SceneID::FTR, "1. Play Game");
    menuScene->registerItem(SceneID::MINS, "2. Instructions");
    menuScene->registerItem(SceneID::NONE, "3. Exit");

}

void GameEngine::createMenuInstuc()
{
    // create the menu_scene and put in sceneMap
    auto menuScene = std::make_shared<Scene_Menu>(this);
    m_sceneMap[SceneID::MINS] = menuScene;

    // add items to menu_scene
    menuScene->registerItem(SceneID::NONE, "Accelerate: UP");
    menuScene->registerItem(SceneID::NONE, "Move right: RIGHT");
    menuScene->registerItem(SceneID::NONE, "Move left:  LEFT");
    menuScene->registerItem(SceneID::NONE, " ");
    menuScene->registerItem(SceneID::MENU, "Back to menu");
}


void GameEngine::createFactories() {
    m_factories[SceneID::MENU] = std::function<Sptr()>(
        [this]() -> Sptr {
            return std::make_shared<Scene_Menu>(this);
        });

    m_factories[SceneID::MINS] = std::function<Sptr()>(
        [this]() -> Sptr {
            return std::make_shared<Scene_Menu>(this);
        });

    m_factories[SceneID::GEO] = std::function<Sptr()>(
        [this]() -> Sptr {
            return std::make_shared<Scene_Game>(this, "../assets/level1.txt");
        });

    m_factories[SceneID::FTR] = std::function<Sptr()>(
        [this]() -> Sptr {
            return  std::make_shared<Scene_GexFighter>(this, "../assets/gexFighters.txt");
        });
}


void GameEngine::init(const std::string& configPath) {

    // get the window size
    std::ifstream config(configPath);
    if (config.fail()) {
        std::cerr << "Open file " << configPath << " failed\n";
        config.close();
        exit(1);
    }

    std::string token{ "" };
    config >> token;
    while (config) {
        while (config) {
            if (token == "Window") {
                config >> m_windowSize.x >> m_windowSize.y;
            }
            else if (token == "Sound") {
                std::string key;
                std::string path;
                config >> key >> path;

                SoundPlayer::getInstance().loadBuffer(key, path);
            }
            config >> token;
        }
    }
    config.close();
}



void GameEngine::run() {

    sf::Clock clock;
    sf::Time timeSinceLastUpdate{ sf::Time::Zero };

    while (isRunning()) {

        sUserInput();

        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > TIME_PER_FRAME) {
            timeSinceLastUpdate -= TIME_PER_FRAME;

            sUserInput();
            currentScene()->update(TIME_PER_FRAME);				// update world
        }

        updateStatistics(elapsedTime);
        currentScene()->sRender();

        m_window.setView(m_window.getDefaultView());
        m_window.draw(m_statisticsText);
        m_window.display();
    }
}



void GameEngine::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            quit();

        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            if (currentScene()->getActionMap().contains(event.key.code))
            {
                std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap().at(event.key.code), actionType));
            }
        }
    }
}


void GameEngine::updateStatistics(sf::Time dt) {
    m_statisticsUpdateTime += dt;
    m_statisticsNumFrames += 1;
    if (m_statisticsUpdateTime >= sf::seconds(1.0f)) {
        // m_statisticsText.setString("FPS: " + std::to_string(m_statisticsNumFrames));
        m_statisticsUpdateTime -= sf::seconds(1.f);
        m_statisticsNumFrames = 0;
    }
}


void GameEngine::changeScene(SceneID id, bool endCurrentScene) {

    if (id == SceneID::NONE)
        return;

    if (endCurrentScene)
        m_sceneMap.erase(m_currentScene);

    if (!m_sceneMap.contains(id))
        m_sceneMap[id] = m_factories[id]();

    m_currentScene = id;

    MusicPlayer::getInstance().stop();


    if (id == SceneID::MENU)
        MusicPlayer::getInstance().play("menuTheme");
    if (id == SceneID::FTR)
        MusicPlayer::getInstance().play("gameTheme");
}


std::shared_ptr<Scene> GameEngine::currentScene() {
    return m_sceneMap.at(m_currentScene);
}


void GameEngine::quit() {
    m_window.close();
}


sf::RenderWindow& GameEngine::window() {
    return m_window;
}


const Assets& GameEngine::assets() const {
    return m_assets;
}


bool GameEngine::isRunning() {
    return (m_isRunning && m_window.isOpen());
}

void  GameEngine::quitLevel() {
    changeScene(SceneID::MENU, true);
}

void    GameEngine::backLevel() {
    changeScene(SceneID::MENU, false);
}