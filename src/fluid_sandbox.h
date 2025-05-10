#ifndef FLUID_SANDBOX_H
#define FLUID_SANDBOX_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <tuple>
#include <unordered_map>
#include <algorithm>

#include "particle.h"
#include "object.h"
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
inline constexpr float PLASTICITY_DEFAULT = 0.2f;
inline constexpr float YIELD_RATIO_DEFAULT = 0.2f;
inline constexpr float SPRING_STIFFNESS_DEFAULT = 0.0f;
inline constexpr float CONTROL_RADIUS_DEFAULT = 50.0f;
inline constexpr float OBJECT_RADIUS_DEFAULT = 30.0f;
inline constexpr float OBJECT_MASS_DEFAULT = 10.0f;
inline constexpr float PARTICLE_SPAWN_RATE_DEFAULT = 3.0f;
inline constexpr float BASE_PARTICLE_SIZE_DEFAULT = 5.0f;
inline constexpr float PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT = 7.0f;
inline constexpr float BASE_PARTICLE_COLOR_DEFAULT = 255.0f;
inline constexpr float PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT = 125.0f;

constexpr size_t CIRCLE_DRAW_SEGMENTS = 20;

/**
 * @brief Structure holding all tunable parameters for the fluid simulation.
 */
struct SimulationParameters
{
    // Physics parameters
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

    // Controls parameters
    float control_radius = CONTROL_RADIUS_DEFAULT;
    float particle_spawn_rate = PARTICLE_SPAWN_RATE_DEFAULT;
    float object_radius = OBJECT_RADIUS_DEFAULT;
    float object_mass = OBJECT_MASS_DEFAULT;

    // Visuals parameters
    float base_particle_size = BASE_PARTICLE_SIZE_DEFAULT;
    float particle_stress_size_multiplier = PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT;
    float base_particle_color = BASE_PARTICLE_COLOR_DEFAULT;
    float particle_stress_color_multiplier = PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT;
};

/**
 * @brief Main class for the fluid simulation sandbox.
 */
class FluidSandbox : public sf::Drawable
{
public:
    /**
     * @brief Constructs the FluidSandbox.
     * @param size The size of the simulation area.
     */
    FluidSandbox(sf::Vector2u size) : size_(size) {}

    /**
     * @brief Gets the number of particles in the simulation.
     * @return Number of particles in the simulation.
     */
    size_t particle_count() const { return particles_.size(); }

    /**
     * @brief Gets the number of objects in the simulation.
     * @return Number of objects in the simulation.
     */
    size_t object_count() const { return objects_.size(); }

    /**
     * @brief Gets the size of the simulation area.
     * @return Size of the simulation area.
     */
    sf::Vector2u size() const { return size_; }

    /**
     * @brief Gets the simulation parameters.
     * @return Reference to the simulation parameters.
     */
    SimulationParameters &params() { return params_; }

    /**
     * @brief Resizes the simulation area.
     * @param size The new size of the simulation area.
     */
    void resize(sf::Vector2u size) { size_ = size; }

    /**
     * @brief Clears all particles and objects.
     */
    void clear();

    /**
     * @brief Adds a new particle to the simulation.
     * @param position The position of the new particle.
     */
    void add_particles(sf::Vector2f position);

    /**
     * @brief Adds a new object to the simulation.
     * @param position The position of the new object.
     */
    void add_object(sf::Vector2f position);

    /**
     * @brief Removes particles or objects at a given position.
     * @param position The position to remove particles or objects from.
     */
    void remove_particles(sf::Vector2f position);

    /**
     * @brief Removes an object at a given position.
     * @param position The position to remove the object from.
     */
    void remove_object(sf::Vector2f position);

    /**
     * @brief Toggles the locked state of an object at a given position.
     * @param position The position of the object to toggle.
     */
    void toggle_lock_object(sf::Vector2f position);

    /**
     * @brief Pushes all particles and objects in a given direction.
     * @param velocity The velocity vector to push everything.
     */
    void push_everything(sf::Vector2f velocity);

    /**
     * @brief Updates the simulation state by time step.
     * (implementation of algorithm 1, section 3. Simulation Step)
     * @param dt Time step.
     */
    void update(float dt);

    /**
     * @brief Draws the current state of the simulation to a render target.
     * @param target The render target.
     * @param states Current render states.
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Vector2u size_;
    SimulationParameters params_;

    float dt_ = 0.0f;

    bool reverse_calculation_order_ = false; // If true, the order of some calculations is reversed (improves stability)

    std::vector<Particle> particles_;
    std::vector<Object> objects_;

    SpatialHashGrid<Particle> particle_grid_;
    SpatialHashGrid<Object> object_grid_;
    float max_object_radius = 0.0f;

    std::vector<std::vector<Particle *>> particle_neighbors_;

    /**
     * @brief Moves all particles and objects based on their velocities.
     */
    void move_everything();

    /**
     * @brief Updates the neighbors of each particle using the spatial hash grid.
     */
    void update_neighbors();

    /**
     * @brief Simulation of elasticity (Implementation of algorithms 3 and 4, section 5. Viscoelasticity).
     */
    void adjust_apply_strings();

    /**
     * @brief The core of the fluid simulation (Implementation of algorithm 2, section 4. Double density relaxation).
     */
    void do_double_density_relaxation();

    /**
     * @brief Resolves collisions between particles, objects and simulation boundaries.
     */
    void resolve_collisions();

    /**
     * @brief Recalculates the velocity based on previous position and current position.
     */
    void recalculate_velocity();

    /**
     * @brief Applies gravity to all particles and objects.
     */
    void apply_gravity();

    /**
     * @brief Simulation of viscosity (Implementation of algorithm 5, section 5. Viscoelasticity).
     */
    void apply_viscosity();
};
#endif
