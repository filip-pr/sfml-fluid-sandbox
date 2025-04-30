
#ifndef UTILS_H
#define UTILS_H

#include <SFML/Graphics.hpp>

#include <cmath>

namespace utils
{
    inline float distance(sf::Vector2f a, sf::Vector2f b)
    {
        return std::hypot(a.x - b.x, a.y - b.y);
    }

    inline float distance_sq(sf::Vector2f a, sf::Vector2f b)
    {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    }
}

#endif
