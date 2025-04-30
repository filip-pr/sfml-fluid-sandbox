

#include "fluid_sandbox.h"
#include "utils.h"

#include <algorithm>

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
    for (auto &&particle : particles_)
    {
        if (particle.position.x - PARTICLE_RADIUS < 0)
        {
            particle.position.x = PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x * dampening_factor;
        }
        if (particle.position.x + PARTICLE_RADIUS > size_.x)
        {
            particle.position.x = size_.x - PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x * dampening_factor;
        }
        if (particle.position.y - PARTICLE_RADIUS < 0)
        {
            particle.position.y = PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y * dampening_factor;
        }
        if (particle.position.y + PARTICLE_RADIUS > size_.y)
        {
            particle.position.y = size_.y - PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y * dampening_factor;
        }
    }
}

void FluidSandbox::apply_double_density_relaxation()
{
    float interaction_radius = 30;

    float rest_density = 4.0f;
    float stiffness = 0.5f;
    float near_stiffness = 0.5f;

    for (auto &&particle : particles_)
    {
        float density = 0;
        float near_density = 0;
        auto neighbors = grid_.query(particle.position, interaction_radius);

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;
            float distance_ratio = utils::distance(particle.position, neighbor->position) / interaction_radius;
            if (distance_ratio >= 1)
                continue;
            density += std::pow(1 - distance_ratio, 2);
            near_density += std::pow(1 - distance_ratio, 3);
        }

        float pressure = stiffness * (density - rest_density);
        float near_pressure = near_stiffness * near_density;

        sf::Vector2f force = {0, 0};

        for (auto &&neighbor : neighbors)
        {
            if (neighbor == &particle)
                continue;
            float distance_ratio = utils::distance(neighbor->position, particle.position) / interaction_radius;
            if (distance_ratio >= 1)
                continue;
            auto distance_vector = utils::distance_vector(neighbor->position, particle.position) + sf::Vector2f(0.0001f, 0.0001f);
            distance_vector /= distance_vector.length();
            sf::Vector2f displacement = distance_vector * static_cast<float>(std::pow(dt_, 2) * (pressure * (1 - distance_ratio) + near_pressure * std::pow(1 - distance_ratio, 2)) / 2);

            neighbor->position += displacement;

            force -= displacement;
        }

        particle.position += force;
    }
}

void FluidSandbox::recalculate_velocity()
{
    for (auto &&particle : particles_)
    {
        particle.velocity = (particle.position - particle.prev_position) / dt_;
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
