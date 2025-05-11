## Project Documentation

- **C++ implementation of 2D particle based fluid simulation using SFML.**
- **Any references to algorithms and sections are references to the paper [Particle-based viscoelastic fluid simulation by Simon Clavet, Philippe Beaudoin, and Pierre Poulin](https://dl.acm.org/doi/10.1145/1073368.1073400) as its algorithms are the base of this project.**

### File: `src/controls.h`

#### Struct `Param`
*   **Description:** Represents a simulation parameter that can be changed.
*   **Members:**
    *   `name`: `std::string`
    *   `key`: `char`
    *   `default_value`: `float`
    *   `value`: `float &`
    *   `step_size`: `float`
    *   `min_value`: `float` (default: `std::numeric_limits<float>::lowest()`)
    *   `max_value`: `float` (default: `std::numeric_limits<float>::max()`)
*   **Methods:**
    *   `convert_key(char key)`: Converts a character key to an `sf::Keyboard::Key`.
    *   `update(float dt)`: Updates the parameter's value based on keyboard input. Handles incrementing, decrementing, reseting, and clamping.

#### Class `ControlsDisplay`
*   **Inherits:** `sf::Drawable`
*   **Description:** Manages and displays the simulation controls and parameter information.
*   **Public Methods:**
    *   `ControlsDisplay(FluidSandbox &sandbox, unsigned int width)`: Constructs the `ControlsDisplay`.
    *   `update(float dt)`: Updates the state of all parameters.
    *   `draw(sf::RenderTarget &target, sf::RenderStates states) const override`: Draws the controls and parameter information.
*   **Private Members:**
    *   `sandbox_`: `FluidSandbox &`
    *   `font_`: `sf::Font`
    *   `width_`: `unsigned int`
    *   `dt_`: `float`
    *   `params_`: `std::vector<Param>`
*   **Private Methods:**
    *   `draw_text(...)`: Helper function to draw a line of text.
    *   `draw_info(const std::string &text, float value, ...)`: Helper function to draw an informational line (name and value).
    *   `draw_info(const Param &param, ...)`: Helper function to draw information for a `Param` struct.

---
### File: `src/fluid_sandbox.h`

#### Struct `SimulationParameters`
*   **Description:** Structure holding all tunable parameters for the fluid simulation.
*   **Members (Examples):**
    *   **Physics:** `simulation_speed`, `gravity_x`, `gravity_y`, `edge_bounciness`, `interaction_radius`, `rest_density`, `stiffness`, `near_stiffness`, `linear_viscosity`, `quadratic_viscosity`, `plasticity`, `yield_ratio`, `spring_stiffness`.
    *   **Controls:** `control_radius`, `particle_spawn_rate`, `object_radius`, `object_mass`.
    *   **Visuals:** `base_particle_size`, `particle_stress_size_multiplier`, `base_particle_color`, `particle_stress_color_multiplier`.

#### Class `FluidSandbox`
*   **Inherits:** `sf::Drawable`
*   **Description:** Main class for the fluid simulation sandbox.
*   **Public Methods:**
    *   `FluidSandbox(sf::Vector2u size)`: Constructs the `FluidSandbox`.
    *   `particle_count() const`: Gets the number of particles.
    *   `object_count() const`: Gets the number of objects.
    *   `size() const`: Gets the size of the simulation area.
    *   `params()`: Gets the simulation parameters.
    *   `resize(sf::Vector2u size)`: Resizes the simulation area.
    *   `clear()`: Clears all particles and objects.
    *   `add_particles(sf::Vector2f position)`: Adds new particles.
    *   `add_object(sf::Vector2f position)`: Adds a new object.
    *   `remove_particles(sf::Vector2f position)`: Removes particles or objects at a position.
    *   `remove_object(sf::Vector2f position)`: Removes an object at a position.
    *   `toggle_lock_object(sf::Vector2f position)`: Toggles the locked state of an object.
    *   `try_grab_object(sf::Vector2f position)`: Attempts to grab an object.
    *   `push_everything(sf::Vector2f velocity)`: Pushes all particles and objects.
    *   `update(float dt)`: Updates the simulation state (implementation of algorithm 1, section 3. Simulation Step from the paper).
    *   `draw(sf::RenderTarget &target, sf::RenderStates states) const override`: Draws the current state of the simulation.
*   **Private Methods (References to algorithms in the paper):**
    *   `move_everything()`: Moves all particles and objects.
    *   `update_neighbors()`: Updates neighbors of each particle.
    *   `adjust_apply_strings()`: Simulation of elasticity (Algorithms 3 and 4, section 5. Viscoelasticity).
    *   `do_double_density_relaxation()`: Core fluid simulation (Algorithm 2, section 4. Double density relaxation).
    *   `resolve_collisions()`: Resolves collisions (Algorithm 6, section 6. Collisions).
    *   `recalculate_velocity()`: Recalculates velocity.
    *   `apply_gravity()`: Applies gravity.
    *   `apply_viscosity()`: Simulation of viscosity (Algorithm 5, section 5. Viscoelasticity).

---
### File: `src/object.h`

#### Struct `Object`
*   **Description:** Represents a generic circular rigid body in the fluid simulation.
*   **Members:**
    *   `position`: `sf::Vector2f`
    *   `previous_position`: `sf::Vector2f`
    *   `radius`: `float`
    *   `mass`: `float`
    *   `velocity`: `sf::Vector2f`
    *   `velocity_buffer`: `sf::Vector2f`
    *   `is_locked`: `bool` (default: `false`)
*   **Methods:**
    *   `Object(sf::Vector2f position, float radius, float mass, sf::Vector2f velocity = {0.0f, 0.0f})`: Constructs a new `Object`.
    *   `update(float dt)`: Updates the object's position if not locked.
    *   `toggle_lock()`: Toggles the locked state and resets velocity.

---
### File: `src/particle.h`

#### Struct `Particle`
*   **Description:** Represents a single particle in the fluid simulation.
*   **Members:**
    *   `id`: `size_t` (unique, auto-incremented)
    *   `position`: `sf::Vector2f`
    *   `prev_position`: `sf::Vector2f`
    *   `velocity`: `sf::Vector2f`
    *   `springs`: `std::unordered_map<size_t, float>` (Key: other particle ID, Value: resting length of spring. Used for viscoelasticity.)
    *   `stress`: `float` (Represents stress for visualization, smoothed.)
*   **Methods:**
    *   `Particle(sf::Vector2f position, sf::Vector2f velocity = {0.0f, 0.0f})`: Constructs a new `Particle`.
    *   `update(float dt)`: Updates the particle's position.

---
### File: `src/spatial_hash_grid.h`

#### Class `SpatialHashGrid<T>`
*   **Template Parameter:** `T` (Type of objects to store, must have `sf::Vector2f position`)
*   **Description:** A spatial hash grid for efficient neighbor searching.
*   **Public Methods:**
    *   `update(std::vector<T> &objects, size_t cell_size)`: Updates grid with objects and cell size.
    *   `query(sf::Vector2f center, float radius) const`: Queries for objects within a radius.
*   **Private Methods:**
    *   `insert(std::vector<T> &objects)`: Inserts objects into the grid.
    *   `clear()`: Clears all objects from the grid.
    *   `hash_position(sf::Vector2f position) const`: Computes hash key for a position.
    *   `hash_cell(size_t cell_x, size_t cell_y) const`: Computes hash key for cell coordinates.

---
### File: `src/utils.h`

#### Namespace `utils`
*   **Description:** Contains utility functions for the simulation.
*   **Constants:**
    *   `HASH_PRIME`: `constexpr size_t` (Prime number for hashing)
*   **Functions:**
    *   `distance_sq(sf::Vector2f a, sf::Vector2f b)`: Calculates squared distance between two 2D vectors.
    *   `dot_product(const sf::Vector2<T> &a, const sf::Vector2<T> &b)`: Calculates dot product of two 2D vectors.
