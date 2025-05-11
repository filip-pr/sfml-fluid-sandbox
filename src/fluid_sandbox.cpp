#include <algorithm>
#include <cmath>

#include "fluid_sandbox.h"
#include "utils.h"
#include "controls.h"


void FluidSandbox::clear()
{
    particles_.clear();
    objects_.clear();
}

void FluidSandbox::add_particles(sf::Vector2f position)
{
    size_t num_new_particles = static_cast<size_t>(params_.particle_spawn_rate * dt_);
    if (num_new_particles == 0) // If the whole number of particles is 0, we spawn one on random chance
    {
        num_new_particles = static_cast<float>(rand()) / RAND_MAX < params_.particle_spawn_rate * dt_ ? 1 : 0;
    }
    particles_.reserve(particles_.size() + num_new_particles);
    for (size_t i = 0; i < num_new_particles; ++i)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float distance = static_cast<float>(rand()) / RAND_MAX * params_.control_radius;
        sf::Vector2f offset = {std::cos(angle) * distance, std::sin(angle) * distance};
        particles_.emplace_back(position + offset);
    }
}

void FluidSandbox::add_object(sf::Vector2f position)
{
    // Only spawn new object if it doesn't collide with any existing object
    auto neighbors = object_grid_.query(position, max_object_radius + params_.object_radius);
    for (auto &&neighbor : neighbors)
    {
        float radius_sum = neighbor->radius + params_.object_radius;
        if (utils::distance_sq(neighbor->position, position) < radius_sum * radius_sum)
        {
            return;
        }
    }
    objects_.emplace_back(position, params_.object_radius, params_.object_mass);
}

void FluidSandbox::remove_particles(sf::Vector2f position)
{
    float radius_sq = params_.control_radius * params_.control_radius;
    auto it = std::remove_if(particles_.begin(), particles_.end(),
                             [position, radius_sq](const Particle &particle)
                             {
                                 return utils::distance_sq(particle.position, position) < radius_sq;
                             });
    particles_.erase(it, particles_.end());
}

void FluidSandbox::remove_object(sf::Vector2f position)
{
    auto it = std::remove_if(objects_.begin(), objects_.end(),
                             [position](const Object &object)
                             {
                                 return utils::distance_sq(object.position, position) < object.radius * object.radius;
                             });
    objects_.erase(it, objects_.end());
}

void FluidSandbox::toggle_lock_object(sf::Vector2f position)
{
    auto neighbors = object_grid_.query(position, max_object_radius + params_.object_radius);
    for (auto &&object : objects_)
    {
        float radius_sq = object.radius * object.radius;
        if (utils::distance_sq(object.position, position) < radius_sq)
        {
            object.toggle_lock();
        }
    }
}

std::optional<Object *> FluidSandbox::try_grab_object(sf::Vector2f position)
{
    auto it = std::find_if(objects_.begin(), objects_.end(),
                           [position](const Object &object)
                           {
                               return utils::distance_sq(object.position, position) < object.radius * object.radius;
                           });
    if (it != objects_.end())
    {
        return &(*it);
    }
    return std::nullopt;
}

void FluidSandbox::push_everything(sf::Vector2f velocity)
{
    for (auto &&particle : particles_)
    {
        particle.velocity += velocity;
    }
    for (auto &&object : objects_)
    {
        object.velocity += velocity;
    }
}

void FluidSandbox::update(float dt)
{
    dt_ = std::min(dt * params_.simulation_speed, 1.0f); // to prevent instability (some calculations use higher power of dt)
    move_everything();
    update_neighbors();
    adjust_apply_strings();
    do_double_density_relaxation();
    resolve_collisions();
    recalculate_velocity();
    apply_gravity();
    apply_viscosity();
    reverse_calculation_order_ = !reverse_calculation_order_; // Reverse the order of calculations for better stability
}

void FluidSandbox::move_everything()
{
    for (auto &&particle : particles_)
    {
        particle.update(dt_);
    }
    particle_grid_.update(particles_, params_.interaction_radius);

    max_object_radius = 0.0f;
    for (auto &&object : objects_)
    {
        object.update(dt_);
        object.velocity_buffer = {0.0f, 0.0f};
        if (object.radius > max_object_radius)
        {
            max_object_radius = object.radius;
        }
    }
    object_grid_.update(objects_, max_object_radius);
}

void FluidSandbox::update_neighbors()
{
    if (particles_.size() != particle_neighbors_.size())
    {
        particle_neighbors_.resize(particles_.size());
    }
    size_t particle_id = 0;
    for (auto &&particle : particles_)
    {
        particle_neighbors_[particle_id] = particle_grid_.query(particle.position, params_.interaction_radius);
        ++particle_id;
    }
}

void FluidSandbox::adjust_apply_strings()
{
    if (params_.spring_stiffness == 0.0f) // If spring stiffness is 0, no forces would be applied anyway
        return;

    // Precalculating some values for efficiency
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_plasticity = params_.plasticity * dt_;
    const float dt_sq_spring_stiffness_half = params_.spring_stiffness * dt_ * dt_ * 0.5f;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];

        auto &neighbors = particle_neighbors_[particle_id];

        std::unordered_map<size_t, float> new_springs;
        new_springs.reserve(neighbors.size());

        for (auto &&neighbor : neighbors)
        {
            if (neighbor <= &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }
            float distance = std::sqrt(distance_sq);
            float spring_length;

            auto it = particle.springs.find(neighbor->id);
            if (it != particle.springs.end())
            {
                spring_length = it->second;
            }
            else
            {
                spring_length = params_.interaction_radius;
            }
            float tolerable_deformation = spring_length * params_.yield_ratio;
            if (distance > spring_length + tolerable_deformation)
            {
                spring_length += dt_plasticity * (distance - spring_length - tolerable_deformation);
            }
            else if (distance < spring_length - tolerable_deformation)
            {
                spring_length -= dt_plasticity * (spring_length - distance - tolerable_deformation);
            }
            if (spring_length > params_.interaction_radius)
            {
                continue;
            }
            new_springs.emplace(neighbor->id, spring_length);

            float displacement_magnitude = dt_sq_spring_stiffness_half * (1 - spring_length * inv_interaction_radius) * (spring_length - distance) / distance;

            sf::Vector2f displacement = (neighbor->position - particle.position) * displacement_magnitude;

            particle.position -= displacement;
            neighbor->position += displacement;
        }
        std::swap(particle.springs, new_springs);
    }
}

void FluidSandbox::do_double_density_relaxation()
{
    // Precalculating some values for efficiency
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_sq_half = 0.5f * dt_ * dt_;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];
        float density = 0.0f;
        float near_density = 0.0f;

        auto &neighbors = particle_neighbors_[particle_id];

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance * inv_interaction_radius;

            float one_minus_ratio = 1.0f - distance_ratio;
            float one_minus_ratio_sq = one_minus_ratio * one_minus_ratio;

            density += one_minus_ratio_sq;
            near_density += one_minus_ratio_sq * one_minus_ratio;
        }

        float pressure = params_.stiffness * (density - params_.rest_density);
        float near_pressure = params_.near_stiffness * near_density;

        particle.stress = STRESS_SMOOTHING * particle.stress + (1 - STRESS_SMOOTHING) * near_pressure;

        sf::Vector2f total_displacement = {0.0f, 0.0f};

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            sf::Vector2f position_diff = neighbor->position - particle.position;
            float distance_sq = position_diff.lengthSquared();

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance * inv_interaction_radius;
            float one_minus_ratio = 1.0f - distance_ratio;

            float displacement_magnitude = dt_sq_half * (pressure * one_minus_ratio + near_pressure * (one_minus_ratio * one_minus_ratio)) / distance;

            sf::Vector2f displacement = position_diff * displacement_magnitude;

            neighbor->position += displacement;
            total_displacement -= displacement;
        }
        particle.position += total_displacement;
    }
}

void FluidSandbox::resolve_collisions()
{
    const float min_x = 0;
    const float max_x = static_cast<float>(size_.x);
    const float min_y = 0;
    const float max_y = static_cast<float>(size_.y);

    // Particle boundary collisions
    for (auto &&particle : particles_)
    {
        if (particle.position.x < min_x)
        {
            particle.position.x = min_x;
            particle.velocity.x *= -params_.edge_bounciness;
        }
        else if (particle.position.x > max_x)
        {
            particle.position.x = max_x;
            particle.velocity.x *= -params_.edge_bounciness;
        }

        if (particle.position.y < min_y)
        {
            particle.position.y = min_y;
            particle.velocity.y *= -params_.edge_bounciness;
        }
        else if (particle.position.y > max_y)
        {
            particle.position.y = max_y;
            particle.velocity.y *= -params_.edge_bounciness;
        }
        if (std::isnan(particle.position.x))
        {
            particle.position.x = 0;
        }
        if (std::isnan(particle.position.y))
        {
            particle.position.y = 0;
        }
    }

    // Particle object collisions
    for (auto &object : objects_)
    {
        if (object.is_locked)
        {
            continue;
        }

        auto coliding_particles = particle_grid_.query(object.position, object.radius);

        for (auto particle : coliding_particles)
        {

            float distance_sq = utils::distance_sq(object.position, particle->position);

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = particle->position - object.position;
                particle->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);

            sf::Vector2f collision_normal = (object.position - particle->position) / distance;

            float inward_velocity = utils::dot_product(object.velocity - particle->velocity, collision_normal);

            if (inward_velocity < 0)
            {
                float mass_ratio = object.mass / (object.mass + 1.0f); // Particle mass is implicitly 1.0f
                object.velocity_buffer -= collision_normal * inward_velocity * mass_ratio / object.mass;
            }
            // sqrt here is necessary to prevent particles too much inside the object to push it too much
            object.velocity_buffer += collision_normal * std::sqrt(object.radius - distance) / object.mass;
        }
        object.velocity += object.velocity_buffer;
        object.position = object.previous_position + object.velocity * dt_;
    }

    // Inter object and object boundary collisions
    for (auto &&object : objects_)
    {
        auto neighbors = object_grid_.query(object.position, object.radius + max_object_radius);
        for (auto &&neighbor : neighbors)
        {
            if (&object == neighbor || object.is_locked && neighbor->is_locked)
            {
                continue;
            }

            float distance_sq = utils::distance_sq(object.position, neighbor->position);

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - object.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float radius_sum = object.radius + neighbor->radius;

            if (distance_sq >= radius_sum * radius_sum)
            {
                continue;
            }

            float distance = std::sqrt(distance_sq);

            sf::Vector2f collision_normal = (object.position - neighbor->position) / distance;
            float inward_velocity = utils::dot_product(object.velocity - neighbor->velocity, collision_normal);

            float overlap = radius_sum - distance;

            if (object.is_locked)
            {
                neighbor->position -= collision_normal * overlap;
                if (inward_velocity < 0)
                {
                    neighbor->velocity += collision_normal * inward_velocity;
                }
            }
            else if (neighbor->is_locked)
            {
                object.position += collision_normal * overlap;
                if (inward_velocity < 0)
                {
                    object.velocity -= collision_normal * inward_velocity;
                }
            }
            else
            {
                float mass_ratio = object.mass / (object.mass + neighbor->mass);
                object.position += collision_normal * overlap * mass_ratio;
                neighbor->position -= collision_normal * overlap * (1.0f - mass_ratio);
                if (inward_velocity < 0)
                {
                    object.velocity -= collision_normal * inward_velocity * mass_ratio;
                    neighbor->velocity += collision_normal * inward_velocity * (1.0f - mass_ratio);
                }
            }
        }

        if (object.is_locked)
        {
            continue;
        }

        if (object.position.x - object.radius < min_x)
        {
            object.position.x = min_x + object.radius;
            object.velocity.x *= -params_.edge_bounciness;
        }
        else if (object.position.x + object.radius > max_x)
        {
            object.position.x = max_x - object.radius;
            object.velocity.x *= -params_.edge_bounciness;
        }

        if (object.position.y - object.radius < min_y)
        {
            object.position.y = min_y + object.radius;
            object.velocity.y *= -params_.edge_bounciness;
        }
        else if (object.position.y + object.radius > max_y)
        {
            object.position.y = max_y - object.radius;
            object.velocity.y *= -params_.edge_bounciness;
        }
    }

    // Object particle collisions
    for (auto &object : objects_)
    {
        auto coliding_particles = particle_grid_.query(object.position, object.radius);

        for (auto particle : coliding_particles)
        {

            float distance_sq = utils::distance_sq(object.position, particle->position);

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = particle->position - object.position;
                particle->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);

            sf::Vector2f collision_normal = (object.position - particle->position) / distance;

            float inward_velocity = utils::dot_product(object.velocity - particle->velocity, collision_normal);

            if (inward_velocity < 0)
            {
                float mass_ratio = object.mass / (object.mass + 1.0f); // Particle mass is implicitly 1.0f
                particle->velocity += collision_normal * inward_velocity * (1.0f - mass_ratio);
            }
            particle->position -= collision_normal * (object.radius - distance);
        }
    }
}

void FluidSandbox::recalculate_velocity()
{
    const float inv_dt = 1.0f / dt_;
    for (auto &&particle : particles_)
    {
        particle.velocity = (particle.position - particle.prev_position) * inv_dt;
    }
}

void FluidSandbox::apply_gravity()
{
    for (auto &&particle : particles_)
    {
        particle.velocity.x += params_.gravity_x * dt_;
        particle.velocity.y += params_.gravity_y * dt_;
    }
    for (auto &&object : objects_)
    {
        object.velocity.x += params_.gravity_x * dt_;
        object.velocity.y += params_.gravity_y * dt_;
    }
}

void FluidSandbox::apply_viscosity()
{
    if (params_.linear_viscosity == 0.0f && params_.quadratic_viscosity == 0.0f) // If both viscosities are 0, no forces would be applied anyway
        return;

    // Precalculating some values for efficiency
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_half = 0.5f * dt_;

    size_t num_particles = particles_.size();

    for (size_t i = 0; i < num_particles; ++i)
    {
        size_t particle_id = reverse_calculation_order_ ? num_particles - i - 1 : i;
        auto &particle = particles_[particle_id];

        auto &neighbors = particle_neighbors_[particle_id];

        for (auto &&neighbor : neighbors)
        {
            if (neighbor <= &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq)
                continue;

            if (distance_sq < 0.01f)
            {
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            sf::Vector2f position_diff = (neighbor->position - particle.position);
            float non_normal_inward_velocity = utils::dot_product((particle.velocity - neighbor->velocity), position_diff);

            if (non_normal_inward_velocity > 0.0f)
            {
                float distance = std::sqrt(distance_sq);
                float inward_velocity = std::min(non_normal_inward_velocity / distance, 1.0f);

                float impulse_magnitude = dt_half * (1 - distance * inv_interaction_radius) * inward_velocity * (params_.linear_viscosity + params_.quadratic_viscosity * inward_velocity) / distance;

                sf::Vector2f impulse = position_diff * impulse_magnitude;

                particle.velocity -= impulse;
                neighbor->velocity += impulse;
            }
        }
    }
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // Draw particles as squares
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];
        float particle_size = std::max(params_.base_particle_size + particle.stress * params_.particle_stress_size_multiplier, 1.0f);
        int pressure_color = std::clamp(static_cast<int>(params_.base_particle_color - particle.stress * params_.particle_stress_color_multiplier), 0, 255);
        sf::Color particle_color = sf::Color(pressure_color, pressure_color, 255);

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f({particle_size, -particle_size});
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-particle_size, particle_size);
        for (size_t j = 0; j < 6; j++)
        {
            particle_vertices[i * 6 + j].color = particle_color;
        }
    }

    states.blendMode = sf::BlendMax;
    target.draw(particle_vertices, states);
    states.blendMode = sf::BlendAlpha;

    // Draw objects as circles
    sf::VertexArray object_vertices(sf::PrimitiveType::Triangles, objects_.size() * CIRCLE_DRAW_SEGMENTS * 3);

    for (size_t i = 0; i < objects_.size(); ++i)
    {
        const auto &object = objects_[i];
        sf::Color object_color = object.is_locked ? sf::Color(128, 0, 0) : sf::Color(0, 128, 0);

        sf::Vector2f center_pos = object.position;

        for (size_t j = 0; j < CIRCLE_DRAW_SEGMENTS; ++j)
        {
            float angle1 = static_cast<float>(j) / CIRCLE_DRAW_SEGMENTS * 2.0f * M_PI;
            float angle2 = static_cast<float>(j + 1) / CIRCLE_DRAW_SEGMENTS * 2.0f * M_PI;

            sf::Vector2f p1 = object.position + sf::Vector2f(std::cos(angle1) * object.radius, std::sin(angle1) * object.radius);
            sf::Vector2f p2 = object.position + sf::Vector2f(std::cos(angle2) * object.radius, std::sin(angle2) * object.radius);

            size_t vertex_idx = (i * CIRCLE_DRAW_SEGMENTS + j) * 3;

            object_vertices[vertex_idx].position = object.position;
            object_vertices[vertex_idx + 1].position = p1;
            object_vertices[vertex_idx + 2].position = p2;

            object_vertices[vertex_idx].color = object_color;
            object_vertices[vertex_idx + 1].color = object_color;
            object_vertices[vertex_idx + 2].color = object_color;
        }
    }
    target.draw(object_vertices, states);
}
