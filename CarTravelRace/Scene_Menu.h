#ifndef SFMLCLASS_SCENE_MENU_H
#define SFMLCLASS_SCENE_MENU_H

#include "Scene.h"
#include <vector>

class Scene_Menu : public Scene
{
private:
    std::vector<std::pair<SceneID, std::string>>	m_menuItems;
    sf::Text					                    m_menuText;
    std::vector<std::string>	                    m_levelPaths;
    int							                    m_menuIndex{ 0 };
    std::string					                    m_title;


    void                        init();
    void                        onEnd() override;
public:

    Scene_Menu(GameEngine* gameEngine);

    void        registerItem(SceneID key, std::string item);
    void		update(sf::Time dt) override;
    void		sDoAction(const Action& action) override;
    void		sRender() override;

};




#endif //SFMLCLASS_SCENE_MENU_H
