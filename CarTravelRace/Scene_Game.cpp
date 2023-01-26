#include <SFML/System/Time.hpp>
#include <fstream>
#include "Entity.h"
#include "Scene_Game.h"
#include "Action.h"
#include "GameEngine.h"
#include "Components.h"


Scene_Game::Scene_Game(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine), m_levelPath(levelPath) {

    init(m_levelPath);
    spawnPlayer();
}


void Scene_Game::init(const std::string& configPath) {
    loadFromFile(configPath);
    registerActions();
}


void Scene_Game::loadFromFile(const std::string& configPath) {
    std::ifstream config(configPath);
    if (config.fail()) {
        std::cerr << "Open file " << configPath << " failed\n";
        config.close();
        exit(1);
    }


    std::string token{ "" };
    config >> token;
    while (config) {
        if (token == "#") {
            std::string tmp;
            std::getline(config, tmp);
            std::cout << tmp << "\n";
        }
        else if (token == "Circle") {

            auto e = m_entityManager.addEntity("shape");

            sf::Vector2f pos, vel;
            float rd;
            int r, g, b;
            std::string label;

            config >> label >> rd >> pos.x >> pos.y >> vel.x
                >> vel.y >> r >> g >> b;

            e->addComponent<CShape>(rd, sf::Color(r, g, b));
            e->addComponent<CTransform>(pos, vel);
            e->addComponent<CCollision>(rd);
        }
        config >> token;
    }
    config.close();
}


void Scene_Game::sDoAction(const Action& action) {
    // On Key Press
    if (action.type() == "START")
    {

        if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
        else if (action.name() == "QUIT") { m_game->quitLevel(); }
        else if (action.name() == "BACK") { m_game->backLevel(); }
        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

        // Player control
        else if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }
    }


    // on Key Release
    else if (action.type() == "END")
    {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = false; }
    }

}


void Scene_Game::update(sf::Time dt) {

    m_entityManager.update();

    if (m_isPaused)
        return;

    sMovement(dt);
    sCollisions();
}


void Scene_Game::sRender() {

    // draw world

    auto bgColor = sf::Color(100, 100, 255);
    if (m_isPaused)
        bgColor = sf::Color(150, 50, 255);

    m_game->window().clear(bgColor);


    for (auto e : m_entityManager.getEntities()) {

        // draw the world
        if (e->getComponent<CShape>().has) {
            auto& tfm = e->getComponent<CTransform>();
            auto& shape = e->getComponent<CShape>().shape;
            shape.setPosition(tfm.pos);
            shape.setRotation(tfm.rot);
            m_game->window().draw(shape);
        }

        if (e->getComponent<CSprite>().has) {
            auto& tfm = e->getComponent<CTransform>();
            auto& sprite = e->getComponent<CSprite>().sprite;
            sprite.setPosition(tfm.pos);
            sprite.setRotation(tfm.rot);
            m_game->window().draw(sprite);
        }

    }

    if (m_isPaused) {
        sf::Text paused("PAUSED", m_game->assets().getFont("Megaman"), 128);
        centerOrigin(paused);
        auto bounds = getViewBounds();
        paused.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        m_game->window().draw(paused);
    }

    // draw bounding boxes
    if (m_drawAABB) {
        drawAABB();
    }


}


void Scene_Game::onEnd() {
    m_game->changeScene(SceneID::MENU, false);
}

void Scene_Game::sMovement(sf::Time dt) {

    // player movement
    sf::Vector2f pv;
    auto& pInput = m_player->getComponent<CInput>();
    if (pInput.left) pv.x -= 1;
    if (pInput.right) pv.x += 1;
    if (pInput.up) pv.y -= 1;
    if (pInput.down) pv.y += 1;

    float playerSpeed = 500.f;
    pv = normalize(pv);
    m_player->getComponent<CTransform>().vel = playerSpeed * pv;



    // move all objects
    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CTransform>()) {
            auto& tfm = e->getComponent<CTransform>();

            tfm.pos += tfm.vel * dt.asSeconds();
            tfm.rot += tfm.rotVel * dt.asSeconds();
        }
    }
}


void Scene_Game::sCollisions() {
    keepEntitiesInBounds();
    adjustPlayerPosition();

    //
    // collisions between player and shape
    //
    auto playerPos = m_player->getComponent<CTransform>().pos;
    auto& playerCR = m_player->getComponent<CCollision>().radius;
    auto& playerShape = m_player->getComponent<CShape>().shape;


    for (auto e : m_entityManager.getEntities("shape")) {

        auto shapePos = e->getComponent<CTransform>().pos;
        auto shapeCR = e->getComponent<CCollision>().radius;

        if (length(shapePos - playerPos) < shapeCR + playerCR) {
            float sr = playerShape.getRadius();
            playerShape.setRadius(sr + e->getComponent<CShape>().shape.getRadius());
            // playerCR += shapeCR;
            centerOrigin(playerShape);
            e->destroy();
        }
    }
}


void Scene_Game::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
    registerAction(sf::Keyboard::Q, "QUIT");

    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");

    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");

}


void Scene_Game::adjustPlayerPosition() {
    auto vb = getViewBounds();
    auto& pos = m_player->getComponent<CTransform>().pos;
    auto cr = m_player->getComponent<CCollision>().radius;

    pos.x = std::max(pos.x, vb.left + cr);
    pos.x = std::min(pos.x, vb.left + vb.width - cr);
    pos.y = std::max(pos.y, vb.top + cr);
    pos.y = std::min(pos.y, vb.top + vb.height - cr);

}


void Scene_Game::keepEntitiesInBounds() {
    auto vb = getViewBounds();

    for (auto e : m_entityManager.getEntities()) {

        if (e->hasComponent<CCollision>()) {
            auto& tfm = e->getComponent<CTransform>();
            auto r = e->getComponent<CCollision>().radius;

            if (tfm.pos.x - r < vb.left || (tfm.pos.x + r) >(vb.left + vb.width)) {
                tfm.vel.x *= -1;
            }
            if (tfm.pos.y - r < vb.top || (tfm.pos.y + r) >(vb.top + vb.height)) {
                tfm.vel.y *= -1;
            }
        }
    }
}



void Scene_Game::drawAABB() {

    for (auto e : m_entityManager.getEntities()) {

        if (e->hasComponent<CCollision>()) {
            auto pos = e->getComponent<CTransform>().pos;
            auto cr = e->getComponent<CCollision>().radius;

            sf::CircleShape circ;
            circ.setRadius(cr);
            centerOrigin(circ);

            circ.setPosition(pos);
            circ.setFillColor(sf::Color(0, 0, 0, 0));
            circ.setOutlineColor(sf::Color(0, 0, 0));
            circ.setOutlineThickness(5.f);
            m_game->window().draw(circ);
        }
    }
}


void Scene_Game::sUpdate(sf::Time dt) {
    // FOR LATER
}


void Scene_Game::spawnPlayer() {

    auto bounds = getViewBounds();

    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(
        sf::Vector2f(bounds.left = bounds.width / 2.f, bounds.top + bounds.height / 2.f),
        sf::Vector2f(0.f, 0.f),
        0, 0);

    auto& sprite = m_player->addComponent<CSprite>(m_game->assets().getTexture("Entities")).sprite;

    sprite.setTextureRect(sf::IntRect(96, 0, 48, 64));

    m_player->addComponent<CCollision>(15);
    m_player->addComponent<CInput>();

}


sf::FloatRect Scene_Game::getViewBounds() {
    auto view = m_game->window().getView();

    return sf::FloatRect{
            {view.getCenter().x - view.getSize().x / 2.f, view.getCenter().y - view.getSize().y / 2.f},
            {view.getSize().x, view.getSize().y} };

}
