#include "fluid_sandbox.h"
#include "utils.h"

#include <algorithm>
#include <cmath>

void FluidSandbox::add_particle(sf::Vector2f position, sf::Vector2f velocity)
{
    particles_.emplace_back(position, velocity);
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
    if (particle_neighbors_.size() < particles_.size())
    {
        particle_neighbors_.resize(particles_.size());
    }
    apply_gravity();
    apply_viscosity();
    move_particles();
    adjust_springs();
    apply_spring_displacements();

    size_t particle_id = 0;
    for (auto &&particle : particles_)
    {
        particle_neighbors_[particle_id] = grid_.query(particle.position, params_.interaction_radius);
        ++particle_id;
    }

    do_double_density_relaxation();
    resolve_collisions();
    recalculate_velocity();
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
}

void FluidSandbox::move_particles()
{
    for (auto &&particle : particles_)
    {
        particle.update(params_.dt);
    }
    grid_.update(particles_, params_.interaction_radius);
}

void FluidSandbox::adjust_springs()
{
}

void FluidSandbox::apply_spring_displacements()
{
}

void FluidSandbox::do_double_density_relaxation()
{
    const float interaction_radius_sq = params_.interaction_radius * params_.interaction_radius;
    const float dt_sq_half = 0.5f * params_.dt * params_.dt;

    size_t particle_id = 0;
    for (auto &&particle : particles_)
    {
        float density = 0.0f;
        float near_density = 0.0f;

        auto neighbors = particle_neighbors_[particle_id];

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq || distance_sq < 0.0001f)
                continue;

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance / params_.interaction_radius;

            float one_minus_ratio = 1.0f - distance_ratio;
            float one_minus_ratio_sq = one_minus_ratio * one_minus_ratio;

            density += one_minus_ratio;
            near_density += one_minus_ratio * one_minus_ratio;
        }

        float pressure = params_.stiffness * (density - params_.rest_density);
        float near_pressure = params_.near_stiffness * near_density;

        if (particle_id >= particle_pressures_.size())
        {
            particle_pressures_.push_back(pressure);
        }
        else
        {
            particle_pressures_[particle_id] = pressure;
        }

        sf::Vector2f total_displacement = {0.0f, 0.0f};

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            sf::Vector2f position_diff = neighbor->position - particle.position;
            float dist_sq = position_diff.lengthSquared();

            if (dist_sq >= interaction_radius_sq || dist_sq < 0.0001f)
                continue;

            float distance = std::sqrt(dist_sq);
            float distance_ratio = distance / params_.interaction_radius;
            float one_minus_ratio = 1.0f - distance_ratio;

            float displacement_magnitude = dt_sq_half * (pressure * one_minus_ratio + near_pressure * (one_minus_ratio * one_minus_ratio));

            sf::Vector2f displacement = position_diff * (displacement_magnitude / distance);

            neighbor->position += displacement;
            total_displacement -= displacement;
        }
        particle.position += total_displacement;
        ++particle_id;
    }
}

void FluidSandbox::resolve_collisions()
{
    float dampening_factor = 0.3f;
    const float min_x = PARTICLE_RADIUS;
    const float max_x = static_cast<float>(size_.x) - PARTICLE_RADIUS;
    const float min_y = PARTICLE_RADIUS;
    const float max_y = static_cast<float>(size_.y) - PARTICLE_RADIUS;

    for (auto &&particle : particles_)
    {
        if (particle.position.x < min_x)
        {
            particle.position.x = min_x;
            particle.velocity.x *= -dampening_factor;
        }
        else if (particle.position.x > max_x)
        {
            particle.position.x = max_x;
            particle.velocity.x *= -dampening_factor;
        }

        if (particle.position.y < min_y)
        {
            particle.position.y = min_y;
            particle.velocity.y *= -dampening_factor;
        }
        else if (particle.position.y > max_y)
        {
            particle.position.y = max_y;
            particle.velocity.y *= -dampening_factor;
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

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];
        float pressure = particle_pressures_[i];
        float particle_size = std::max(15 + pressure * 5, 1.0f);

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f(particle_size, -particle_size);
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-particle_size, -particle_size);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(particle_size, particle_size);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-particle_size, particle_size);
        for (size_t j = 0; j < 6; j++)
        {
            int pressure_color = std::clamp(static_cast<int>(128 - pressure * 50), 0, 255);
            particle_vertices[i * 6 + j].color = sf::Color(pressure_color, pressure_color, 255);
        }
    }

    target.draw(particle_vertices, states);
}
