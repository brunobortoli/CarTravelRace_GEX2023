#ifndef SFMLCLASS_ACTION_H
#define SFMLCLASS_ACTION_H

#include <string>
#include <SFML/System/Vector2.hpp>

struct Action {
    std::string m_name{ "none" };
    std::string m_type{ "none" };
    sf::Vector2f m_pos{ 0.f, 0.f };


    Action() = default;
    Action(const std::string& name, const std::string& type, sf::Vector2f pos = sf::Vector2f(0.f, 0.f));

    const std::string& name() const;
    const std::string& type() const;
    const sf::Vector2f& pos() const;

    std::string toString() const;
};


#endif //SFMLCLASS_ACTION_H
