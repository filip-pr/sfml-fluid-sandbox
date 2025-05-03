
#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>

#include "particle.h"
#include "spatial_hash_grid.h"

constexpr float PARTICLE_RADIUS = 3.0f;
constexpr float GRAVITY = 0.4f;

struct SimulationParameters
{
    float dt;
    sf::Vector2f gravity;
    float interaction_radius;
    float rest_density;
    float stiffness;
    float near_stiffness;
};

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size, SimulationParameters &params) : size_(size), params_(params) {}

    size_t particle_count() const { return particles_.size(); }
    void resize(sf::Vector2u size) { size_ = size; }
    void update_params(SimulationParameters &params) { params_ = params; }

    void add_particle(sf::Vector2f position, sf::Vector2f velocity = {0, 0});
    void push_particles(sf::Vector2f velocity);

    void update();
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    SimulationParameters params_;

    std::vector<Particle> particles_;
    SpatialHashGrid grid_;

    std::vector<float> particle_pressures_;
    std::vector<std::vector<Particle *>> particle_neighbors_;

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
