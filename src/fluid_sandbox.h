
#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>

#include "particle.h"
#include "spatial_hash_grid.h"

constexpr float PARTICLE_RADIUS = 5;
constexpr float GRAVITY = 0.4f;

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size, float dt) : size_(size), dt_(dt) {}

    void add_particle(sf::Vector2f position, sf::Vector2f velocity);
    void add_particle(sf::Vector2f position);

    size_t particle_count() const { return particles_.size(); }

    void resize(sf::Vector2u size) { size_ = size; }

    void update();
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    float dt_;
    std::vector<Particle> particles_;
    SpatialHashGrid grid_{static_cast<size_t>(PARTICLE_RADIUS) * 4};

    void apply_gravity();
    void move_particles();
    void enforce_constraints();
    void apply_double_density_relaxation();
    void recalculate_velocity();
};
#endif
