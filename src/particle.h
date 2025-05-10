#ifndef PARTICLE_H
#define PARTICLE_H

#include <SFML/Graphics.hpp>

#include <unordered_map>

constexpr float STRESS_SMOOTHING = 0.7f; // Smoothing factor to prevent flickering from changing computation order.

/**
 * @brief Represents a single particle in the fluid simulation.
 */
struct Particle
{
private:
    static size_t id_counter;

public:
    size_t id = id_counter++;
    sf::Vector2f position;
    sf::Vector2f prev_position;
    sf::Vector2f velocity;

    /**
     * @brief Stores springs connected to this particle.
     * The key is the ID of the other particle, and the value is the resting length of the spring.
     * Used to model viscoelasticity.
     */
    std::unordered_map<size_t, float> springs;

    float stress = 0.0f; // Represents the stress experienced by the particle, used only for visualization.

    /**
     * @brief Constructs a new Particle.
     * @param position Initial position of the particle.
     * @param velocity Initial velocity of the particle (defaults to zero).
     */
    Particle(sf::Vector2f position, sf::Vector2f velocity = {0.0f, 0.0f}) : position(position), prev_position(position), velocity(velocity) {}

    /**
     * @brief Updates the particle's position based on its velocity and the time step.
     * @param dt Time step.
     */
    inline void update(float dt)
    {
        prev_position = position;
        position += velocity * dt;
    }
};

inline size_t Particle::id_counter = 0;

#endif
