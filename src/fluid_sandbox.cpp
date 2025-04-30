#include "fluid_sandbox.h"
#include "utils.h"

#include <algorithm>
#include <cmath>

void FluidSandbox::add_particle(sf::Vector2f position, sf::Vector2f velocity)
{
    particles_.emplace_back(position, velocity);
    grid_.insert(&particles_.back());
}

void FluidSandbox::add_particle(sf::Vector2f position)
{
    particles_.emplace_back(position);
    grid_.insert(&particles_.back());
}

void FluidSandbox::apply_gravity()
{
    for (auto &&particle : particles_)
    {
        particle.velocity.y += GRAVITY * dt_;
    }
}

void FluidSandbox::move_particles()
{
    grid_.clear();
    for (auto &&particle : particles_)
    {
        particle.update(dt_);
    }
    grid_.batch_insert(particles_);
}

void FluidSandbox::enforce_constraints()
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

void FluidSandbox::apply_double_density_relaxation()
{
    const float interaction_radius = 60.0f;

    const float rest_density = 6.0f;
    const float stiffness = 0.5f;
    const float near_stiffness = 0.5f;

    const float interaction_radius_sq = interaction_radius * interaction_radius;
    const float dt_sq_half = 0.5f * dt_ * dt_;

    for (auto &&particle : particles_)
    {
        float density = 0.0f;
        float near_density = 0.0f;
        auto neighbors = grid_.query(particle.position, interaction_radius);

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;

            float distance_sq = utils::distance_sq(particle.position, neighbor->position);

            if (distance_sq >= interaction_radius_sq || distance_sq < 0.0001f)
                continue;

            float distance = std::sqrt(distance_sq);
            float distance_ratio = distance / interaction_radius;

            float one_minus_ratio = 1.0f - distance_ratio;
            float one_minus_ratio_sq = one_minus_ratio * one_minus_ratio;

            density += one_minus_ratio;
            near_density += one_minus_ratio * one_minus_ratio;
        }

        float pressure = stiffness * (density - rest_density);
        float near_pressure = near_stiffness * near_density;

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
            float distance_ratio = distance / interaction_radius;
            float one_minus_ratio = 1.0f - distance_ratio;

            float displacement_magnitude = dt_sq_half * (pressure * one_minus_ratio + near_pressure * (one_minus_ratio * one_minus_ratio));

            sf::Vector2f displacement = position_diff * (displacement_magnitude / distance);

            neighbor->position += displacement;
            total_displacement -= displacement;
        }
        particle.position += total_displacement;
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

void FluidSandbox::update()
{
    apply_gravity();
    move_particles();
    apply_double_density_relaxation();
    enforce_constraints();
    recalculate_velocity();
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, PARTICLE_RADIUS);
        for (size_t j = 0; j < 6; j++)
        {
            particle_vertices[i * 6 + j].color = sf::Color::Blue;
        }
    }

    target.draw(particle_vertices, states);
}
