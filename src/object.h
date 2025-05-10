#ifndef OBJECT_H
#define OBJECT_H

#include <SFML/Graphics.hpp>

/**
 * @brief Represents a generic circular rigid body in the fluid simulation.
 */
struct Object
{
public:
    sf::Vector2f position;
    float radius;
    float mass;
    sf::Vector2f velocity;
    bool is_locked = false;

    /**
     * @brief Constructs a new Object.
     * @param position Initial position of the object.
     * @param radius Radius of the object.
     * @param mass Mass of the object.
     * @param velocity Initial velocity of the object (defaults to zero).
     */
    Object(sf::Vector2f position, float radius, float mass, sf::Vector2f velocity = {0.0f, 0.0f})
        : position(position), radius(radius), mass(mass), velocity(velocity) {}

    /**
     * @brief Updates the object's position based on its velocity and the time step.
     * Does nothing if the object is locked.
     * @param dt Time step.
     */
    void update(float dt)
    {
        if (!is_locked)
        {
            position += velocity * dt;
        }
    }

    /**
     * @brief Toggles the locked state of the object.
     */
    void toggle_lock()
    {
        is_locked = !is_locked;
        velocity = {0.0f, 0.0f}; // Reset velocity when locking/unlocking
    }
};

#endif
