#ifndef UTILS_H
#define UTILS_H

#include <SFML/Graphics.hpp>

#include <cmath>

/**
 * @brief Contains utility functions for the simulation.
 */
namespace utils
{
    /**
     * @brief A prime number used for hashing in the spatial hash grid.
     */
    constexpr size_t HASH_PRIME = 1168639;

    /**
     * @brief Calculates the squared distance between two 2D vectors.
     * @param a The first vector.
     * @param b The second vector.
     * @return The squared distance between a and b.
     */
    inline float distance_sq(sf::Vector2f a, sf::Vector2f b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    /**
     * @brief Calculates the dot product of two 2D vectors.
     * @tparam T The type of the vector components.
     * @param a The first vector.
     * @param b The second vector.
     * @return The dot product of a and b.
     */
    template <typename T>
    inline T dot_product(const sf::Vector2<T> &a, const sf::Vector2<T> &b)
    {
        return a.x * b.x + a.y * b.y;
    }

}

#endif
