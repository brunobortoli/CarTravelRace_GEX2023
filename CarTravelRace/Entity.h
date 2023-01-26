#ifndef SFMLCLASS_ENTITY_H
#define SFMLCLASS_ENTITY_H

#include <string>
#include <tuple>
#include <memory>
#include "Components.h"

// forward declarations
class EntityManager;

using ComponentTuple = std::tuple<CSprite, CShape, CTransform, CState,
    CCollision, CHealth, CInput, CGun, CAnimation, CMissiles, CAutoPilot>;

class Entity {
private:
    friend class EntityManager;
    Entity(size_t id, const std::string& tag);

    const size_t        m_id{ 0 };
    const std::string   m_tag{ "none" };
    bool                m_active{ true };
    ComponentTuple      m_components;


public:
    void                destroy();
    const size_t& getId() const;
    const std::string& getTag() const;
    bool                isActive() const;


    template<typename T>
    T& getComponent() {
        return std::get<T>(m_components);
    }

    template<typename T>
    bool hasComponent() {
        return getComponent<T>().has;
    }

    template<typename T, typename... TArgs>
    T& addComponent(TArgs &&... mArgs) {
        auto& component = getComponent<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.has = true;
        return component;
    }


    template<typename T>
    bool removeComponent() {
        return getComponent<T>().has = false;
    }

};


#endif //SFMLCLASS_ENTITY_H

