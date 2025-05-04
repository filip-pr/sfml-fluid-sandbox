
#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <tuple>
#include <unordered_map>

#include "particle.h"
#include "spatial_hash_grid.h"

constexpr float BASE_PARTICLE_SIZE = 5.0f;
constexpr float PARTICLE_STRESS_SIZE_MULTIPLIER = 15.0f;
constexpr float BASE_PARTICLE_COLOR = 220.0f;
constexpr float PARTICLE_STRESS_COLOR_MULTIPLIER = 100.0f;

constexpr float COLLISION_DAMPENING = 1.0f;

struct SimulationParameters
{
    float dt;
    sf::Vector2f gravity;
    float interaction_radius;
    float rest_density;
    float stiffness;
    float near_stiffness;
    float linear_viscosity;
    float quadratic_viscosity;
    float plasticity;
    float yield_ratio;
    float spring_stiffness;
};

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size, SimulationParameters &params) : size_(size), params_(params) {}

    size_t particle_count() const { return particles_.size(); }
    void resize(sf::Vector2u size) { size_ = size; }
    void update_params(SimulationParameters &params) { params_ = params; }

    void clear_particles();
    void add_particles(sf::Vector2f position, float radius, size_t number);
    void remove_particles(sf::Vector2f position, float radius);
    void push_particles(sf::Vector2f velocity);

    void update();
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    SimulationParameters params_;

    bool reverse_calculation_order_ = false;

    std::vector<Particle> particles_;
    std::vector<Particle> new_particles_;
    SpatialHashGrid grid_;

    std::vector<std::vector<Particle *>> particle_neighbors_;

    void move_particles();
    void update_neighbors();
    void adjust_apply_strings();
    void do_double_density_relaxation();
    void resolve_collisions();
    void recalculate_velocity();
    void apply_gravity();
    void apply_viscosity();
};
#endif
