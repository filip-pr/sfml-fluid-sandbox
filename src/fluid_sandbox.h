
#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>

#include "particle.h"
#include "spatial_hash_grid.h"

constexpr float PARTICLE_RADIUS = 3.0f;
constexpr float GRAVITY = 0.4f;

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size, float dt) : size_(size), dt_(dt) {}

    void add_particle(sf::Vector2f position, sf::Vector2f velocity);
    void add_particle(sf::Vector2f position);

    size_t particle_count() const { return particles_.size(); }

    void resize(sf::Vector2u size) { size_ = size; }

    void add_particle_velocity(sf::Vector2f velocity);

    void update();
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    float dt_;
    std::vector<Particle> particles_;
    std::vector<sf::Color> particle_colors_;
    SpatialHashGrid grid_{static_cast<size_t>(PARTICLE_RADIUS) * 10};

    void apply_gravity();
    void apply_viscosity();
    void move_particles();
    void adjust_springs();
    void apply_spring_displacements();
    void do_double_density_relaxation();
    void resolve_collisions();
    void recalculate_velocity();
};
#endif
