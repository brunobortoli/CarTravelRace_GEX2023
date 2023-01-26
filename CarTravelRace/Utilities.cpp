#include <cmath>
#include "Utilities.h"
#include <numbers>

static const float PI = std::numbers::pi;

float length(const sf::Vector2f& v) {
    return std::sqrtf(v.x * v.x + v.y * v.y);
}


float dist(const sf::Vector2f& a, const sf::Vector2f& b) {
    return length(b - a);
}

sf::Vector2f normalize(sf::Vector2f v) {
    static const float epsi = 0.0001;
    float d = length(v);
    if (d > epsi)
        v = v / length(v);

    return v;
}

float radToDeg(float r) {
    return r * 180 / PI;
}

float degToRad(float d) {
    return d * PI / 180;
}

float bearing(const sf::Vector2f& v) {
    return radToDeg(std::atan2(v.y, v.x));
}

sf::Vector2f uVecFromBearing(float d) {
    float r = degToRad(d);
    return sf::Vector2f(std::cos(r), std::sin(r));
}

std::ostream& operator<<(std::ostream& os, sf::Vector2f v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
