//
// Created by David Burchill on 2022-10-21.
//

#include <SFML/Window/Keyboard.hpp>
#include "Scene_Menu.h"
#include "GameEngine.h"
#include "Scene_Game.h"
#include <memory>



Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");

    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");

    registerAction(sf::Keyboard::D, "PLAY");

    registerAction(sf::Keyboard::Escape, "QUIT");

    m_title = "Geo Wars";


    m_menuText.setFont(m_game->assets().getFont("Megaman"));

    const size_t CHAR_SIZE{ 64 };
    m_menuText.setCharacterSize(CHAR_SIZE);

}


void Scene_Menu::registerItem(SceneID key, std::string item) {
    m_menuItems.push_back(std::make_pair(key, item));
}


void Scene_Menu::update(sf::Time dt) {
    m_entityManager.update();
}


void Scene_Menu::sRender() {
    sf::View view = m_game->window().getView();
    view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
    m_game->window().setView(view);

    static const sf::Color selectedColor(255, 255, 255);
    static const sf::Color normalColor(0, 0, 0);
    static const sf::Color backgroundColor(100, 100, 255);

    sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC",
        m_game->assets().getFont("Megaman"),
        20);
    footer.setFillColor(normalColor);
    footer.setPosition(32, 700);

    m_game->window().clear(backgroundColor);

    m_menuText.setFillColor(normalColor);
    m_menuText.setString(m_title);
    m_menuText.setPosition(10, 10);
    m_game->window().draw(m_menuText);

    for (size_t i{ 0 }; i < m_menuItems.size(); ++i)
    {
        m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
        m_menuText.setPosition(32, 32 + (i + 1) * 96);
        m_menuText.setString(m_menuItems.at(i).second);
        m_game->window().draw(m_menuText);
    }

    m_game->window().draw(footer);

}


void Scene_Menu::sDoAction(const Action& action) {
    if (action.type() == "START")
    {
        if (action.name() == "UP")
        {
            m_menuIndex = (m_menuIndex + m_menuItems.size() - 1) % m_menuItems.size();
        }
        else if (action.name() == "DOWN")
        {
            m_menuIndex = (m_menuIndex + 1) % m_menuItems.size();
        }
        // TODO generalize
        else if (action.name() == "PLAY")
        {
            m_game->changeScene(m_menuItems.at(m_menuIndex).first);
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}



void Scene_Menu::onEnd()
{
    m_game->window().close();
}
