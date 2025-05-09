#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <tuple>
#include <unordered_map>
#include <algorithm>

#include "particle.h"
#include "spatial_hash_grid.h"

inline constexpr float SIMULATION_SPEED_DEFAULT = 100.0f;
inline constexpr float GRAVITY_X_DEFAULT = 0.0f;
inline constexpr float GRAVITY_Y_DEFAULT = 0.4f;
inline constexpr float EDGE_BOUNCINESS_DEFAULT = 0.0f;
inline constexpr float INTERACTION_RADIUS_DEFAULT = 60.0f;
inline constexpr float REST_DENSITY_DEFAULT = 6.0f;
inline constexpr float STIFFNESS_DEFAULT = 0.5f;
inline constexpr float NEAR_STIFFNESS_DEFAULT = 0.5f;
inline constexpr float LINEAR_VISCOSITY_DEFAULT = 0.0f;
inline constexpr float QUADRATIC_VISCOSITY_DEFAULT = 0.0f;
inline constexpr float PLASTICITY_DEFAULT = 0.0f;
inline constexpr float YIELD_RATIO_DEFAULT = 0.2f;
inline constexpr float SPRING_STIFFNESS_DEFAULT = 0.5f;
inline constexpr float CONTROL_RADIUS_DEFAULT = 50.0f;
inline constexpr float PARTICLE_SPAWN_RATE_DEFAULT = 3.0f;
inline constexpr float BASE_PARTICLE_SIZE_DEFAULT = 5.0f;
inline constexpr float PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT = 7.0f;
inline constexpr float BASE_PARTICLE_COLOR_DEFAULT = 255.0f;
inline constexpr float PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT = 125.0f;

struct SimulationParameters
{
    float simulation_speed = SIMULATION_SPEED_DEFAULT;
    float gravity_x = GRAVITY_X_DEFAULT;
    float gravity_y = GRAVITY_Y_DEFAULT;
    float edge_bounciness = EDGE_BOUNCINESS_DEFAULT;
    float interaction_radius = INTERACTION_RADIUS_DEFAULT;
    float rest_density = REST_DENSITY_DEFAULT;
    float stiffness = STIFFNESS_DEFAULT;
    float near_stiffness = NEAR_STIFFNESS_DEFAULT;
    float linear_viscosity = LINEAR_VISCOSITY_DEFAULT;
    float quadratic_viscosity = QUADRATIC_VISCOSITY_DEFAULT;
    float plasticity = PLASTICITY_DEFAULT;
    float yield_ratio = YIELD_RATIO_DEFAULT;
    float spring_stiffness = SPRING_STIFFNESS_DEFAULT;

    float control_radius = CONTROL_RADIUS_DEFAULT;
    float particle_spawn_rate = PARTICLE_SPAWN_RATE_DEFAULT;

    float base_particle_size = BASE_PARTICLE_SIZE_DEFAULT;
    float particle_stress_size_multiplier = PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT;
    float base_particle_color = BASE_PARTICLE_COLOR_DEFAULT;
    float particle_stress_color_multiplier = PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT;
};

class FluidSandbox : public sf::Drawable
{
public:
    FluidSandbox(sf::Vector2u size) : size_(size) {}

    size_t particle_count() const { return particles_.size(); }
    sf::Vector2u size() const { return size_; }
    SimulationParameters &params() { return params_; }
    void resize(sf::Vector2u size) { size_ = size; }

    void clear_particles();
    void add_particles(sf::Vector2f position);
    void remove_particles(sf::Vector2f position);
    void push_particles(sf::Vector2f velocity);

    void update(float dt);
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    SimulationParameters params_;
    sf::Font font_;

    float dt_ = 0.0f;

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
