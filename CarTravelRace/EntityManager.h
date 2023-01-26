#ifndef SFMLCLASS_ENTITYMANAGER_H
#define SFMLCLASS_ENTITYMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <string>

//forward declare
class Entity;

using NttPtr = std::shared_ptr<Entity>;
using EntityVec = std::vector<NttPtr>;
using EntityMap = std::map<std::string, EntityVec>;

class EntityManager {
private:
    EntityVec           m_entities;
    EntityMap           m_entityMap;
    size_t              m_totalEntites{ 0 };

    EntityVec           m_entitiesToAdd;

    void removeDeadEntities(EntityVec& v);
public:
    EntityManager();

    NttPtr        addEntity(const std::string& tag);
    EntityVec& getEntities();
    EntityVec& getEntities(const std::string& tag);

    void update();

};


#endif //SFMLCLASS_ENTITYMANAGER_H
