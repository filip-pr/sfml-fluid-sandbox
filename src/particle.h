#ifndef PARTICLE_H
#define PARTICLE_H

#include <SFML/Graphics.hpp>

constexpr float STRESS_SMOOTHING = 0.6f;

struct Particle
{
public:
    sf::Vector2f position;
    sf::Vector2f prev_position;
    sf::Vector2f velocity;

    float stress = 0.0f;

    Particle(sf::Vector2f position) : position(position), prev_position(position), velocity(0, 0) {}
    Particle(sf::Vector2f position, sf::Vector2f velocity) : position(position), prev_position(position), velocity(velocity) {}

    void update(float dt)
    {
        prev_position = position;
        position += velocity * dt;
    }
};

#endif
