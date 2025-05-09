
#ifndef OBJECT_H
#define OBJECT_H

#include <SFML/Graphics.hpp>

constexpr size_t CIRCLE_DRAW_SEGMENTS = 20;

struct Object
{
public:
    sf::Vector2f position;

    float radius;
    float mass;
    sf::Vector2f velocity;
    bool is_locked = false;

    Object(sf::Vector2f position, float radius, float mass, sf::Vector2f velocity = {0.0f, 0.0f})
        : position(position), radius(radius), mass(mass), velocity(velocity) {}

    void update(float dt)
    {
        if (!is_locked)
        {
            position += velocity * dt;
        }
    }

    void toggle_lock()
    {
        is_locked = !is_locked;
        velocity = {0.0f, 0.0f};
    }
};

#endif
