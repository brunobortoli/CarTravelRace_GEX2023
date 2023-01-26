#include "Scene.h"

Scene::Scene(GameEngine* gameEngine) : m_game(gameEngine)
{}


void Scene::doAction(Action action)
{
    this->sDoAction(action);
}


const ActionMap Scene::getActionMap() const
{
    return m_actions;
}

void Scene::registerAction(int inputKey, std::string action)
{
    m_actions[inputKey] = action;
}


void Scene::setPaused(bool paused)
{
    m_isPaused = paused;
}

