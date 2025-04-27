

#include "fluid_sandbox.h"

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

void FluidSandbox::update(float dt)
{
    for (auto &&particle : particles_)
    {
        grid_.remove(&particle);
        particle.velocity.y += GRAVITY * dt;
        particle.update(dt);
        if (particle.position.x - PARTICLE_RADIUS < 0)
        {
            particle.position.x = PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x;
        }
        if (particle.position.x + PARTICLE_RADIUS > size_.x)
        {
            particle.position.x = size_.x - PARTICLE_RADIUS;
            particle.velocity.x = -particle.velocity.x;
        }
        if (particle.position.y - PARTICLE_RADIUS < 0)
        {
            particle.position.y = PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y;
        }
        if (particle.position.y + PARTICLE_RADIUS > size_.y)
        {
            particle.position.y = size_.y - PARTICLE_RADIUS;
            particle.velocity.y = -particle.velocity.y;
        }
        grid_.insert(&particle);
    }
}

void FluidSandbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::VertexArray particle_vertices(sf::PrimitiveType::Triangles, particles_.size() * 6);
    for (size_t i = 0; i < particles_.size(); i++)
    {
        particle_vertices[i * 6].position = particles_[i].position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 1].position = particles_[i].position + sf::Vector2f(PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 2].position = particles_[i].position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 3].position = particles_[i].position + sf::Vector2f(-PARTICLE_RADIUS, -PARTICLE_RADIUS);
        particle_vertices[i * 6 + 4].position = particles_[i].position + sf::Vector2f(PARTICLE_RADIUS, PARTICLE_RADIUS);
        particle_vertices[i * 6 + 5].position = particles_[i].position + sf::Vector2f(-PARTICLE_RADIUS, PARTICLE_RADIUS);
        for (size_t j = 0; j < 6; j++)
        {
            particle_vertices[i * 6 + j].color = sf::Color::White;
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
