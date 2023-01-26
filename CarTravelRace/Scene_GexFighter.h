#ifndef SFMLCLASS_SCENE_GEXFIGHTER_H
#define SFMLCLASS_SCENE_GEXFIGHTER_H

#include "Scene.h"
#include <map>
#include <array>


struct EnemyConfig {
    std::array<float, 5> dirs;
    std::array<sf::Time, 5> times;
};

using String = std::string;

class Scene_GexFighter : public Scene {

private:
    std::shared_ptr<Entity>		    m_player;

    std::map<String, sf::IntRect>   m_textRects;

    sf::View                        m_worldView;
    sf::FloatRect                   m_worldBounds;

    float                           m_scrollSpeed;
    float                           m_playerSpeed;
    float                           m_enemySpeed{ 100.f };
    std::map<String, EnemyConfig>   m_enemyConfig;

    float                           m_bulletSpeed;
    float                           m_missileSpeed;
    sf::Time                        m_fireInterval{ sf::seconds(5) };

    sf::Vector2f                    m_spawnPosition;
    bool						    m_drawTextures{ true };
    bool						    m_drawAABB{ false };
    bool				            m_drawGrid{ false };
    bool                            m_playerWon{ false };
    bool                            m_playerLost{ false };
    float                           m_finishPoint{ 0 };


    void	                        onEnd() override;

    //systems
    void                            sMovement(sf::Time dt);
    void                            sCollisions();
    void                            sUpdate(sf::Time dt);
    void                            sGunUpdate(sf::Time dt);
    void                            sAnimation(sf::Time dt);
    void                            sGuideMissiles(sf::Time dt);
    void                            sAutoPilot(const sf::Time& dt);

    // helper functions
    void                            spawnEnemies();
    sf::Vector2f                    findClosestEnemy(sf::Vector2f mPos);
    void                            spawnEnemy(std::string type, sf::Vector2f pos);
    void                            spawnEnemies(std::string type, float offset, size_t numPlanes);
    void                            createBullet(sf::Vector2f pos, bool isEnemy);
    void                            registerActions();
    void                            init(const std::string& configPath);
    void                            loadFromFile(const std::string& configPath);
    void                            spawnPlayer();
    void                            drawAABB();
    void                            keepEntitiesInBounds();
    void                            adjustPlayer();
    sf::FloatRect                   getViewBounds();
    void                            fireBullet();
    void                            fireMissile();
    void                            checkMissileCollision();
    void                            checkBulletCollision();
    void                            checkPlaneCollision();
    void                            checkPickupCollision();
    void                            checkIfDead(NttPtr e);
    void                            spawnPickup(NttPtr e);


public:
    Scene_GexFighter(GameEngine* gameEngine, const std::string& configPath);

    void		                    update(sf::Time dt) override;
    void		                    sDoAction(const Action& action) override;
    void		                    sRender() override;


    void playerMovement();
    void checkPlayerState();
};


#endif //SFMLCLASS_SCENE_GEXFIGHTER_H
