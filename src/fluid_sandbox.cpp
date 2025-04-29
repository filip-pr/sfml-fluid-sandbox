

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

void FluidSandbox::update_particles()
{
    for (auto &&particle : particles_)
    {
        particle.update(dt_);
    }
}

void FluidSandbox::enforce_constraints()
{
    float dampening_factor = 0.5f;
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
    float interaction_radius = PARTICLE_RADIUS * 10;

    float rest_density = 10.0f;
    float stiffness = 0.5f;
    float near_stiffness = 0.5f;

    for (auto &&particle : particles_)
    {
        float density = 0;
        float near_density = 0;
        auto neighbors = grid_.query(particle.position, interaction_radius);

        for (auto &&neighbor : neighbors)
        {
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
            float distance_ratio = utils::distance(neighbor->position, particle.position) / interaction_radius;
            if (distance_ratio >= 1)
                continue;
            sf::Vector2f displacement = utils::distance_vector(neighbor->position, particle.position) * static_cast<float>(std::pow(dt_, 2) * (pressure * (1 - distance_ratio) + near_pressure * std::pow(1 - distance_ratio, 2)) / 2);


            neighbor->position = neighbor->position + displacement;

            force = force - displacement;
        }

        particle.position = particle.position + force;

    }
}

void FluidSandbox::update()
{
    apply_gravity();

    update_particles();
    enforce_constraints();


    apply_double_density_relaxation();
    grid_.clear();
    grid_.batch_insert(particles_);
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        auto &&particle = particles_[i];

        auto neighbors = grid_.query(particle.position, PARTICLE_RADIUS * 2);

        particle_vertices[i * 6].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 1].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 2].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 3].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 4].position = particle.position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 5].position = particle.position + sf::Vector2f(-PARTICLE_RADIUS, PARTICLE_RADIUS);
        for (size_t j = 0; j < 6; j++)
        {
            particle_vertices[i * 6 + j].color = sf::Color(0, std::min(255, static_cast<int>(neighbors.size() * 10 + 100)), 0);
        }
    }

    auto highlight_particles = grid_.query(close_highlight_position, PARTICLE_RADIUS * 2);

    sf::VertexArray highlight_particles_vertices(sf::PrimitiveType::Triangles, highlight_particles.size() * 6);

    for (size_t i = 0; i < highlight_particles.size(); i++)
    {
        highlight_particles_vertices[i * 6].position = highlight_particles[i]->position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        highlight_particles_vertices[i * 6 + 1].position = highlight_particles[i]->position + sf::Vector2f(PARTICLE_RADIUS, -PARTICLE_RADIUS);
        highlight_particles_vertices[i * 6 + 2].position = highlight_particles[i]->position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        highlight_particles_vertices[i * 6 + 3].position = highlight_particles[i]->position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        highlight_particles_vertices[i * 6 + 4].position = highlight_particles[i]->position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        highlight_particles_vertices[i * 6 + 5].position = highlight_particles[i]->position + sf::Vector2f(-PARTICLE_RADIUS, PARTICLE_RADIUS);
        for (size_t j = 0; j < 6; j++)
        {
            highlight_particles_vertices[i * 6 + j].color = sf::Color::Red;
        }
    }

    target.draw(particle_vertices, states);
    target.draw(highlight_particles_vertices, states);
}
