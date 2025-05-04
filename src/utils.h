
#ifndef UTILS_H
#define UTILS_H

#include <SFML/Graphics.hpp>

#include <cmath>

namespace utils
{
    constexpr size_t HASH_PRIME = 1168639;

    inline float distance(sf::Vector2f a, sf::Vector2f b)
    {
        return std::hypot(a.x - b.x, a.y - b.y);
    }

    inline float distance_sq(sf::Vector2f a, sf::Vector2f b)
    {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    }

    template <typename T>
    T dot_product(const sf::Vector2<T> &a, const sf::Vector2<T> &b)
    {
        return a.x * b.x + a.y * b.y;
    }

    struct TupleHash
    {
        std::size_t operator()(const std::tuple<size_t, size_t> &key) const
        {
            return std::get<0>(key) + HASH_PRIME * std::get<1>(key);
        }
    };
}

#endif
