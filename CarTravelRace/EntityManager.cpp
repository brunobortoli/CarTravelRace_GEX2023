#include "EntityManager.h"
#include "Entity.h"
#include <algorithm>


EntityManager::EntityManager()
    : m_totalEntites(0) {}


NttPtr EntityManager::addEntity(const std::string& tag) {
    // crate an entity
    auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntites++, tag));

    // put it the to add vector
    m_entitiesToAdd.push_back(entity);

    // return pointer to the entituy
    return entity;
}


EntityVec& EntityManager::getEntities() {
    return m_entities;
}


EntityVec& EntityManager::getEntities(const std::string& tag) {
    return m_entityMap[tag];
}


void EntityManager::removeDeadEntities(EntityVec& v) {
    v.erase(std::remove_if(
        v.begin(), v.end(),
        [](auto e) { return !(e->isActive()); }),
        v.end());
}


void EntityManager::update() {

    // delete all dead entities
    removeDeadEntities(m_entities);
    for (auto& [_, v] : m_entityMap) {
        removeDeadEntities(v);
    }

    for (auto e : m_entitiesToAdd) {
        m_entities.push_back(e);
        m_entityMap[e->getTag()].push_back(e);
    }

    m_entitiesToAdd.clear();
}