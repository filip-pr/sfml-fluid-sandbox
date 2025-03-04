
#ifndef PARTICLE_H
#define PARTICLE_H

#include <SFML/Graphics.hpp>

struct Particle
{
public:
    sf::Vector2f position;
    sf::Vector2f velocity;

    Particle(sf::Vector2f position) : position(position), velocity(0, 0) {}
    Particle(sf::Vector2f position, sf::Vector2f velocity) : position(position), velocity(velocity) {}

    void update(float dt) { position += velocity * dt; }
};

#endif
