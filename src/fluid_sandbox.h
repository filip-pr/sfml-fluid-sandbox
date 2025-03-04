
#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>

#include "particle.h"

constexpr float PARTICLE_RADIUS = 5;
constexpr float GRAVITY = 9.8f;

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size) : size_(size) {}

    void add_particle(sf::Vector2f position, sf::Vector2f velocity) { particles_.emplace_back(position, velocity); }
    void add_particle(sf::Vector2f position) { particles_.emplace_back(position); }

    void resize(sf::Vector2u size) { size_ = size; }

    void update(float dt);
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    std::vector<Particle> particles_;
};

#endif
