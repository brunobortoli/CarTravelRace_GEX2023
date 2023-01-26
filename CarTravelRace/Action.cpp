#include "Action.h"

Action::Action(const std::string& name, const std::string& type, sf::Vector2f pos)
    : m_name(name)
    , m_type(type)
    , m_pos(pos)
{
}

const std::string& Action::name() const
{
    return m_name;
}

const std::string& Action::type() const
{
    return m_type;
}

std::string Action::toString() const
{
    return m_name + ":" + m_type + "(" + std::to_string(m_pos.x) + ", " + std::to_string(m_pos.y) + ")";
}
