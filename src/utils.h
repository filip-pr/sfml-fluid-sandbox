#ifndef UTILS_H
#define UTILS_H

#include <SFML/Graphics.hpp>

#include <cmath>

namespace utils
{
    constexpr size_t HASH_PRIME = 1168639;

    inline float distance_sq(sf::Vector2f a, sf::Vector2f b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    template <typename T>
    inline T dot_product(const sf::Vector2<T> &a, const sf::Vector2<T> &b)
    {
        return a.x * b.x + a.y * b.y;
    }

}

#endif
