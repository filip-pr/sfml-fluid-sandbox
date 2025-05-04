#include "fluid_sandbox.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <iostream>

void FluidSandbox::clear_particles()
{
    particles_.clear();
}

void FluidSandbox::add_particles(sf::Vector2f position, float radius, size_t number)
{
    particles_.reserve(particles_.size() + number);
    for (size_t i = 0; i < number; ++i)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float distance = static_cast<float>(rand()) / RAND_MAX * radius;
        sf::Vector2f offset = {std::cos(angle) * distance, std::sin(angle) * distance};
        particles_.emplace_back(position + offset);
    }
}

void FluidSandbox::remove_particles(sf::Vector2f position, float radius)
{
    float radius_sq = radius * radius;
    auto it = std::remove_if(particles_.begin(), particles_.end(),
                             [position, radius_sq](const Particle &particle)
                             {
                                 return utils::distance_sq(particle.position, position) < radius_sq;
                             });
    particles_.erase(it, particles_.end());
}

void FluidSandbox::push_particles(sf::Vector2f velocity)
{
    for (auto &&particle : particles_)
    {
        particle.velocity += velocity;
    }
}

void FluidSandbox::update()
{
    move_particles();
    update_neighbors();
    adjust_apply_strings();
    do_double_density_relaxation();
    resolve_collisions();
    recalculate_velocity();
    apply_gravity();
    apply_viscosity();
    reverse_calculation_order_ = !reverse_calculation_order_;
}

void FluidSandbox::move_particles()
{
    for (auto &&particle : particles_)
    {
        particle.update(params_.dt);
    }
    grid_.update(particles_, params_.interaction_radius);
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
        particle_neighbors_[particle_id] = grid_.query(particle.position, params_.interaction_radius);
        ++particle_id;
    }
}

void FluidSandbox::adjust_apply_strings()
{
    if (params_.spring_stiffness == 0.0f || params_.plasticity == 0.0f)
        return;

    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_plasticity = params_.plasticity * params_.dt;
    const float dt_sq_spring_stiffness_half = params_.spring_stiffness * params_.dt * params_.dt * 0.5f;

    auto old_springs = springs_;
    springs_.clear();

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

            std::tuple<size_t, size_t> spring_key = {particle.id, neighbor->id};

            if (old_springs.find(spring_key) != old_springs.end())
            {
                spring_length = old_springs[spring_key];
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
            springs_.emplace(spring_key, spring_length); // TODO try to optimize this
            sf::Vector2f displacement = dt_sq_spring_stiffness_half * (1 - spring_length * inv_interaction_radius) * (spring_length - distance) * (neighbor->position - particle.position) / distance;
            particle.position -= displacement;
            neighbor->position += displacement;
        }
    }
}

void FluidSandbox::do_double_density_relaxation()
{
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_sq_half = 0.5f * params_.dt * params_.dt;

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

            density += one_minus_ratio;
            near_density += one_minus_ratio * one_minus_ratio;
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
                sf::Vector2f position_diff = neighbor->position - particle.position;
                neighbor->position += {position_diff.x > 0 ? 0.1f : -0.1f, position_diff.y > 0 ? 0.1f : -0.1f};
                continue;
            }

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance * inv_interaction_radius;
            float one_minus_ratio = 1.0f - distance_ratio;

            float displacement_magnitude = dt_sq_half * (pressure * one_minus_ratio + near_pressure * (one_minus_ratio * one_minus_ratio));

            sf::Vector2f displacement = position_diff * (displacement_magnitude / distance);

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

    for (auto &&particle : particles_)
    {
        if (particle.position.x < min_x)
        {
            particle.position.x = min_x;
            particle.velocity.x *= -(1 - COLLISION_DAMPENING);
        }
        else if (particle.position.x > max_x)
        {
            particle.position.x = max_x;
            particle.velocity.x *= -(1 - COLLISION_DAMPENING);
        }

        if (particle.position.y < min_y)
        {
            particle.position.y = min_y;
            particle.velocity.y *= -(1 - COLLISION_DAMPENING);
        }
        else if (particle.position.y > max_y)
        {
            particle.position.y = max_y;
            particle.velocity.y *= -(1 - COLLISION_DAMPENING);
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
}

void FluidSandbox::recalculate_velocity()
{
    const float inv_dt = 1.0f / params_.dt;
    for (auto &&particle : particles_)
    {
        particle.velocity = (particle.position - particle.prev_position) * inv_dt;
    }
}

void FluidSandbox::apply_gravity()
{
    for (auto &&particle : particles_)
    {
        particle.velocity += params_.gravity * params_.dt;
    }
}

void FluidSandbox::apply_viscosity()
{
    if (params_.linear_viscosity == 0.0f && params_.quadratic_viscosity == 0.0f)
        return;

    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float inv_interaction_radius = 1.0f / params_.interaction_radius;
    const float dt_half = 0.5f * params_.dt;

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

                sf::Vector2f impulse = dt_half * (1 - distance * inv_interaction_radius) * inward_velocity * (params_.linear_viscosity + params_.quadratic_viscosity * inward_velocity) * position_diff / distance;
                particle.velocity -= impulse;
                neighbor->velocity += impulse;
            }
        }
    }
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];
        float particle_size = std::max(BASE_PARTICLE_SIZE + particle.stress * PARTICLE_STRESS_SIZE_MULTIPLIER, 1.0f);

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f(particle_size, -particle_size);
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-particle_size, particle_size);
        for (size_t j = 0; j < 6; j++)
        {
            int pressure_color = std::clamp(static_cast<int>(BASE_PARTICLE_COLOR - particle.stress * PARTICLE_STRESS_COLOR_MULTIPLIER), 0, 255);
            particle_vertices[i * 6 + j].color = sf::Color(pressure_color, pressure_color, 255);
        }
    }
    states.blendMode = sf::BlendMax;
    target.draw(particle_vertices, states);
}
