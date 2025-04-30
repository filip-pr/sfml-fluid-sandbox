
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
    FluidSandbox(sf::Vector2u size, float dt) : size_(size) {}

    void add_particle(sf::Vector2f position, sf::Vector2f velocity);
    void add_particle(sf::Vector2f position);

    size_t particle_count() const { return particles_.size(); }

    void resize(sf::Vector2u size) { size_ = size; }

    void add_particle_velocity(sf::Vector2f velocity);

    void update(float dt, sf::Vector2f gravity, float interaction_radius, float rest_density, float stiffness, float near_stiffness);
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    std::vector<Particle> particles_;
    SpatialHashGrid grid_{static_cast<size_t>(PARTICLE_RADIUS) * 10};

    std::vector<float> particle_pressures_;
    std::vector<std::vector<Particle *>> neighbors_cache_;

    void apply_gravity(float dt, sf::Vector2f gravity);
    void apply_viscosity();
    void move_particles(float dt);
    void adjust_springs();
    void apply_spring_displacements();
    void do_double_density_relaxation(float dt, float interaction_radius, float rest_density, float stiffness, float near_stiffness);
    void resolve_collisions();
    void recalculate_velocity(float dt);
};
#endif
