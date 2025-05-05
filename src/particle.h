#ifndef PARTICLE_H
#define PARTICLE_H

#include <SFML/Graphics.hpp>

#include <unordered_map>

constexpr float STRESS_SMOOTHING = 0.6f;

struct Particle
{
private:
    static size_t id_counter;

public:
    size_t id = id_counter++;
    sf::Vector2f position;
    sf::Vector2f prev_position;
    sf::Vector2f velocity;

    std::unordered_map<size_t, float> springs;

    float stress = 0.0f;

    Particle(sf::Vector2f position) : position(position), prev_position(position), velocity(0, 0) {}
    Particle(sf::Vector2f position, sf::Vector2f velocity) : position(position), prev_position(position), velocity(velocity) {}

    inline void update(float dt)
    {
        prev_position = position;
        position += velocity * dt;
    }
};

inline size_t Particle::id_counter = 0;

#endif
